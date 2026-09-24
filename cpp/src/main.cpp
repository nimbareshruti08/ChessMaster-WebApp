// ---------------------------------------------------------------------------
// Chess Master - C++ / SFML desktop chess game
//
// Data structures demonstrated in this project:
//   * 2D ARRAY    -> Board::squares[8][8]      (the chessboard)
//   * LINKED LIST -> MoveLinkedList            (complete move history)
//   * STACK       -> MoveStack                 (previous board states / UNDO)
// ---------------------------------------------------------------------------
#include <cstdlib>
#include <ctime>
#include "GameUI.h"

int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    GameUI ui;
    ui.run();

    return 0;
}
