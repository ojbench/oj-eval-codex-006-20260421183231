#ifndef SERVER_H
#define SERVER_H

#include <cstdlib>
#include <iostream>
#include <vector>
#include <queue>

// Internal storage for the game state
static bool is_mine_[40][40];
static bool visited_[40][40];
static bool marked_[40][40];
static bool wrong_mark_[40][40];  // true if a wrong mark happened here (causes failure and prints 'X')
static int adj_cnt_[40][40];
static int visited_safe_count_;  // number of visited non-mine cells

/*
 * You may need to define some global variables for the information of the game map here.
 * Although we don't encourage to use global variables in real cpp projects, you may have to use them because the use of
 * class is not taught yet. However, if you are member of A-class or have learnt the use of cpp class, member functions,
 * etc., you're free to modify this structure.
 */
int rows;         // The count of rows of the game map. You MUST NOT modify its name.
int columns;      // The count of columns of the game map. You MUST NOT modify its name.
int total_mines;  // The count of mines of the game map. You MUST NOT modify its name. You should initialize this
                  // variable in function InitMap. It will be used in the advanced task.
int game_state;  // The state of the game, 0 for continuing, 1 for winning, -1 for losing. You MUST NOT modify its name.

/**
 * @brief The definition of function InitMap()
 *
 * @details This function is designed to read the initial map from stdin. For example, if there is a 3 * 3 map in which
 * mines are located at (0, 1) and (1, 2) (0-based), the stdin would be
 *     3 3
 *     .X.
 *     ...
 *     ..X
 * where X stands for a mine block and . stands for a normal block. After executing this function, your game map
 * would be initialized, with all the blocks unvisited.
 */
void InitMap() {
  std::cin >> rows >> columns;
  // Read the map
  total_mines = 0;
  game_state = 0;
  visited_safe_count_ = 0;
  for (int i = 0; i < 40; ++i) {
    for (int j = 0; j < 40; ++j) {
      is_mine_[i][j] = false;
      visited_[i][j] = false;
      marked_[i][j] = false;
      wrong_mark_[i][j] = false;
      adj_cnt_[i][j] = 0;
    }
  }
  for (int i = 0; i < rows; ++i) {
    std::string s;
    std::cin >> s;
    for (int j = 0; j < columns; ++j) {
      is_mine_[i][j] = (s[j] == 'X');
      if (is_mine_[i][j]) ++total_mines;
    }
  }
  // Precompute adjacent mine counts
  auto inb = [&](int r, int c) { return r >= 0 && r < rows && c >= 0 && c < columns; };
  const int dr[8] = {-1,-1,-1,0,0,1,1,1};
  const int dc[8] = {-1,0,1,-1,1,-1,0,1};
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      if (is_mine_[i][j]) { adj_cnt_[i][j] = -1; continue; }
      int cnt = 0;
      for (int k = 0; k < 8; ++k) {
        int nr = i + dr[k], nc = j + dc[k];
        if (inb(nr, nc) && is_mine_[nr][nc]) ++cnt;
      }
      adj_cnt_[i][j] = cnt;
    }
  }
}

/**
 * @brief The definition of function VisitBlock(int, int)
 *
 * @details This function is designed to visit a block in the game map. We take the 3 * 3 game map above as an example.
 * At the beginning, if you call VisitBlock(0, 0), the return value would be 0 (game continues), and the game map would
 * be
 *     1??
 *     ???
 *     ???
 * If you call VisitBlock(0, 1) after that, the return value would be -1 (game ends and the players loses) , and the
 * game map would be
 *     1X?
 *     ???
 *     ???
 * If you call VisitBlock(0, 2), VisitBlock(2, 0), VisitBlock(1, 2) instead, the return value of the last operation
 * would be 1 (game ends and the player wins), and the game map would be
 *     1@1
 *     122
 *     01@
 *
 * @param r The row coordinate (0-based) of the block to be visited.
 * @param c The column coordinate (0-based) of the block to be visited.
 *
 * @note You should edit the value of game_state in this function. Precisely, edit it to
 *    0  if the game continues after visit that block, or that block has already been visited before.
 *    1  if the game ends and the player wins.
 *    -1 if the game ends and the player loses.
 *
 * @note For invalid operation, you should not do anything.
 */
void VisitBlock(int r, int c) {
  if (game_state != 0) return;
  if (r < 0 || r >= rows || c < 0 || c >= columns) return;
  if (visited_[r][c] || marked_[r][c]) { game_state = 0; return; }

  auto inb = [&](int rr, int cc) { return rr >= 0 && rr < rows && cc >= 0 && cc < columns; };
  const int dr[8] = {-1,-1,-1,0,0,1,1,1};
  const int dc[8] = {-1,0,1,-1,1,-1,0,1};

  // If it is a mine, immediate failure
  if (is_mine_[r][c]) {
    visited_[r][c] = true;  // so it prints 'X' here
    game_state = -1;
    return;
  }

  // BFS to reveal connected zeros and border numbers
  std::queue<std::pair<int,int>> q;
  auto try_visit = [&](int rr, int cc) {
    if (!inb(rr, cc) || visited_[rr][cc] || marked_[rr][cc]) return;
    if (is_mine_[rr][cc]) return;  // never auto-visit mines
    visited_[rr][cc] = true;
    ++visited_safe_count_;
    if (adj_cnt_[rr][cc] == 0) q.emplace(rr, cc);
  };

  try_visit(r, c);
  while (!q.empty()) {
    auto [cr, cc] = q.front(); q.pop();
    for (int k = 0; k < 8; ++k) {
      int nr = cr + dr[k], nc = cc + dc[k];
      if (!inb(nr, nc)) continue;
      if (is_mine_[nr][nc]) continue;
      if (!visited_[nr][nc] && !marked_[nr][nc]) {
        visited_[nr][nc] = true;
        ++visited_safe_count_;
        if (adj_cnt_[nr][nc] == 0) q.emplace(nr, nc);
      }
    }
  }

  // Check victory
  if (visited_safe_count_ == rows * columns - total_mines) {
    game_state = 1;
  } else {
    game_state = 0;
  }
}

/**
 * @brief The definition of function MarkMine(int, int)
 *
 * @details This function is designed to mark a mine in the game map.
 * If the block being marked is a mine, show it as "@".
 * If the block being marked isn't a mine, END THE GAME immediately. (NOTE: This is not the same rule as the real
 * game) And you don't need to
 *
 * For example, if we use the same map as before, and the current state is:
 *     1?1
 *     ???
 *     ???
 * If you call MarkMine(0, 1), you marked the right mine. Then the resulting game map is:
 *     1@1
 *     ???
 *     ???
 * If you call MarkMine(1, 0), you marked the wrong mine(There's no mine in grid (1, 0)).
 * The game_state would be -1 and game ends immediately. The game map would be:
 *     1?1
 *     X??
 *     ???
 * This is different from the Minesweeper you've played. You should beware of that.
 *
 * @param r The row coordinate (0-based) of the block to be marked.
 * @param c The column coordinate (0-based) of the block to be marked.
 *
 * @note You should edit the value of game_state in this function. Precisely, edit it to
 *    0  if the game continues after visit that block, or that block has already been visited before.
 *    1  if the game ends and the player wins.
 *    -1 if the game ends and the player loses.
 *
 * @note For invalid operation, you should not do anything.
 */
void MarkMine(int r, int c) {
  if (game_state != 0) return;
  if (r < 0 || r >= rows || c < 0 || c >= columns) return;
  if (visited_[r][c] || marked_[r][c]) { game_state = 0; return; }

  if (is_mine_[r][c]) {
    marked_[r][c] = true;  // correct mark
    game_state = 0;
  } else {
    wrong_mark_[r][c] = true;
    game_state = -1;  // immediate failure
  }
}

/**
 * @brief The definition of function AutoExplore(int, int)
 *
 * @details This function is designed to auto-visit adjacent blocks of a certain block.
 * See README.md for more information
 *
 * For example, if we use the same map as before, and the current map is:
 *     ?@?
 *     ?2?
 *     ??@
 * Then auto explore is available only for block (1, 1). If you call AutoExplore(1, 1), the resulting map will be:
 *     1@1
 *     122
 *     01@
 * And the game ends (and player wins).
 */
void AutoExplore(int r, int c) {
  if (game_state != 0) return;
  if (r < 0 || r >= rows || c < 0 || c >= columns) return;
  if (!visited_[r][c]) return;         // only for visited non-mine blocks
  if (is_mine_[r][c]) return;          // cannot auto-explore a mine

  auto inb = [&](int rr, int cc) { return rr >= 0 && rr < rows && cc >= 0 && cc < columns; };
  const int dr[8] = {-1,-1,-1,0,0,1,1,1};
  const int dc[8] = {-1,0,1,-1,1,-1,0,1};

  int need = adj_cnt_[r][c];
  if (need < 0) return;
  int marked_correct = 0;
  std::vector<std::pair<int,int>> neighbors;
  for (int k = 0; k < 8; ++k) {
    int nr = r + dr[k], nc = c + dc[k];
    if (!inb(nr, nc)) continue;
    neighbors.emplace_back(nr, nc);
    if (marked_[nr][nc] && is_mine_[nr][nc]) ++marked_correct;
  }
  if (marked_correct != need) {
    game_state = 0;
    return;  // condition not satisfied
  }
  // Visit all non-mine neighbors that are not visited and not marked
  for (auto [nr, nc] : neighbors) {
    if (!is_mine_[nr][nc] && !visited_[nr][nc] && !marked_[nr][nc]) {
      VisitBlock(nr, nc);
      if (game_state != 0) return;  // in case of unexpected (shouldn't happen here)
    }
  }
  // Victory check already handled in VisitBlock calls, but keep safe
  if (visited_safe_count_ == rows * columns - total_mines) {
    game_state = 1;
  } else {
    game_state = 0;
  }
}

/**
 * @brief The definition of function ExitGame()
 *
 * @details This function is designed to exit the game.
 * It outputs a line according to the result, and a line of two integers, visit_count and marked_mine_count,
 * representing the number of blocks visited and the number of marked mines taken respectively.
 *
 * @note If the player wins, we consider that ALL mines are correctly marked.
 */
void ExitGame() {
  if (game_state == 1) {
    std::cout << "YOU WIN!" << std::endl;
  } else {
    std::cout << "GAME OVER!" << std::endl;
  }
  int marked_correct = 0;
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      if (marked_[i][j] && is_mine_[i][j]) ++marked_correct;
    }
  }
  int marked_count_output = (game_state == 1) ? total_mines : marked_correct;
  std::cout << visited_safe_count_ << " " << marked_count_output << std::endl;
  exit(0);  // Exit the game immediately
}

/**
 * @brief The definition of function PrintMap()
 *
 * @details This function is designed to print the game map to stdout. We take the 3 * 3 game map above as an example.
 * At the beginning, if you call PrintMap(), the stdout would be
 *    ???
 *    ???
 *    ???
 * If you call VisitBlock(2, 0) and PrintMap() after that, the stdout would be
 *    ???
 *    12?
 *    01?
 * If you call VisitBlock(0, 1) and PrintMap() after that, the stdout would be
 *    ?X?
 *    12?
 *    01?
 * If the player visits all blocks without mine and call PrintMap() after that, the stdout would be
 *    1@1
 *    122
 *    01@
 * (You may find the global variable game_state useful when implementing this function.)
 *
 * @note Use std::cout to print the game map, especially when you want to try the advanced task!!!
 */
void PrintMap() {
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      char ch = '?';
      if (game_state == 1) {
        if (is_mine_[i][j]) {
          ch = '@';
        } else if (visited_[i][j]) {
          ch = static_cast<char>('0' + adj_cnt_[i][j]);
        } else {
          ch = '?';
        }
      } else {
        if (wrong_mark_[i][j]) {
          ch = 'X';
        } else if (marked_[i][j] && is_mine_[i][j]) {
          ch = '@';
        } else if (visited_[i][j]) {
          if (is_mine_[i][j]) ch = 'X';
          else ch = static_cast<char>('0' + adj_cnt_[i][j]);
        } else {
          ch = '?';
        }
      }
      std::cout << ch;
    }
    std::cout << std::endl;
  }
}

#endif
