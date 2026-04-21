#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <utility>
#include <vector>
#include <string>
#include <limits>

extern int rows;         // The count of rows of the game map.
extern int columns;      // The count of columns of the game map.
extern int total_mines;  // The count of mines of the game map.

// You MUST NOT use any other external variables except for rows, columns and total_mines.

// Internal client-side view of the board as printed by server
static std::vector<std::string> view_board;

/**
 * @brief The definition of function Execute(int, int, bool)
 *
 * @details This function is designed to take a step when player the client's (or player's) role, and the implementation
 * of it has been finished by TA. (I hope my comments in code would be easy to understand T_T) If you do not understand
 * the contents, please ask TA for help immediately!!!
 *
 * @param r The row coordinate (0-based) of the block to be visited.
 * @param c The column coordinate (0-based) of the block to be visited.
 * @param type The type of operation to a certain block.
 * If type == 0, we'll execute VisitBlock(row, column).
 * If type == 1, we'll execute MarkMine(row, column).
 * If type == 2, we'll execute AutoExplore(row, column).
 * You should not call this function with other type values.
 */
void Execute(int r, int c, int type);

/**
 * @brief The definition of function InitGame()
 *
 * @details This function is designed to initialize the game. It should be called at the beginning of the game, which
 * will read the scale of the game map and the first step taken by the server (see README).
 */
void InitGame() {
  // TODO (student): Initialize all your global variables!
  view_board.assign(rows, std::string(columns, '?'));
  int first_row, first_column;
  std::cin >> first_row >> first_column;
  Execute(first_row, first_column, 0);
}

/**
 * @brief The definition of function ReadMap()
 *
 * @details This function is designed to read the game map from stdin when playing the client's (or player's) role.
 * Since the client (or player) can only get the limited information of the game map, so if there is a 3 * 3 map as
 * above and only the block (2, 0) has been visited, the stdin would be
 *     ???
 *     12?
 *     01?
 */
void ReadMap() {
  view_board.resize(rows);
  for (int i = 0; i < rows; ++i) {
    std::string s;
    std::cin >> s;
    view_board[i] = s;
  }
}

/**
 * @brief The definition of function Decide()
 *
 * @details This function is designed to decide the next step when playing the client's (or player's) role. Open up your
 * mind and make your decision here! Caution: you can only execute once in this function.
 */
void Decide() {
  auto inb = [&](int r, int c) { return r >= 0 && r < rows && c >= 0 && c < columns; };
  const int dr[8] = {-1,-1,-1,0,0,1,1,1};
  const int dc[8] = {-1,0,1,-1,1,-1,0,1};

  // 1) Auto-explore when possible
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      char ch = view_board[i][j];
      if (ch >= '0' && ch <= '8') {
        int need = ch - '0';
        int marked = 0, unknown = 0;
        for (int k = 0; k < 8; ++k) {
          int nr = i + dr[k], nc = j + dc[k];
          if (!inb(nr, nc)) continue;
          if (view_board[nr][nc] == '@') ++marked;
          if (view_board[nr][nc] == '?') ++unknown;
        }
        if (marked == need && unknown > 0) {
          Execute(i, j, 2);
          return;
        }
      }
    }
  }

  // 2) Mark certain mines: when (need - marked) == unknown
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      char ch = view_board[i][j];
      if (ch >= '0' && ch <= '8') {
        int need = ch - '0';
        int marked = 0; std::vector<std::pair<int,int>> unknowns;
        for (int k = 0; k < 8; ++k) {
          int nr = i + dr[k], nc = j + dc[k];
          if (!inb(nr, nc)) continue;
          if (view_board[nr][nc] == '@') ++marked;
          else if (view_board[nr][nc] == '?') unknowns.emplace_back(nr, nc);
        }
        if (!unknowns.empty() && (need - marked) == (int)unknowns.size()) {
          // Mark one unknown mine this turn
          Execute(unknowns[0].first, unknowns[0].second, 1);
          return;
        }
      }
    }
  }

  // 3) Choose a low-risk unknown by simple heuristic
  double best_risk = std::numeric_limits<double>::infinity();
  int best_r = -1, best_c = -1;
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      if (view_board[i][j] != '?') continue;
      double risk = 1.0; // default max risk
      bool has_info = false;
      for (int k = 0; k < 8; ++k) {
        int nr = i + dr[k], nc = j + dc[k];
        if (!inb(nr, nc)) continue;
        char ch = view_board[nr][nc];
        if (ch >= '0' && ch <= '8') {
          int need = ch - '0';
          int marked = 0, unknown = 0;
          for (int t = 0; t < 8; ++t) {
            int ar = nr + dr[t], ac = nc + dc[t];
            if (!inb(ar, ac)) continue;
            if (view_board[ar][ac] == '@') ++marked;
            if (view_board[ar][ac] == '?') ++unknown;
          }
          if (unknown > 0) {
            double local = std::max(0, need - marked) / static_cast<double>(unknown);
            risk = std::min(risk, local);
            has_info = true;
          }
        }
      }
      if (!has_info) risk = 0.5; // no info, neutral risk
      if (risk < best_risk) {
        best_risk = risk;
        best_r = i; best_c = j;
      }
    }
  }
  if (best_r != -1) {
    Execute(best_r, best_c, 0);
    return;
  }

  // 4) Fallback: if no unknowns (shouldn't happen before game end), choose any legal op
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      if (view_board[i][j] == '@') continue;
      Execute(i, j, 2);
      return;
    }
  }
}

#endif
