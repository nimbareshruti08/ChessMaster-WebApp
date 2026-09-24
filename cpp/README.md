# Chess Master — C++ / SFML Desktop Chess Game

A complete, fully playable desktop chess game written **only in C++** with an SFML
graphical interface. No web technologies are used anywhere in the game.

## Build and run

```bash
cd ChessMaster
cmake -S . -B build
cmake --build build
./build/bin/ChessMaster          # Windows: build\bin\ChessMaster.exe
```

SFML 2.6.x is used. If it is not installed on your system, CMake downloads and
builds it automatically (requires an internet connection on the first build).

Linux users may need the usual SFML dependencies:
`sudo apt install libfreetype-dev libx11-dev libxrandr-dev libudev-dev libgl1-mesa-dev libxcursor-dev`

## Project structure

```
ChessMaster/
├── CMakeLists.txt
├── include/   ChessGame.h Board.h Piece.h Move.h ChessAI.h
│              MoveLinkedList.h MoveStack.h Timer.h GameUI.h
├── src/       main.cpp + one .cpp per class
└── assets/fonts/DejaVuSans.ttf   (font + Unicode chess piece glyphs)
```

## Required data structures

| Structure   | Where | Used for |
|-------------|-------|----------|
| 2D Array    | `Board::squares[8][8]` (`Board.h`) | the chessboard |
| Linked List | `MoveLinkedList` (`MoveLinkedList.h/.cpp`) | complete move history shown in the GUI |
| Stack       | `MoveStack` (`MoveStack.h/.cpp`) | previous board states for UNDO |

Both the linked list and the stack are hand-written with dynamically allocated
nodes (`new` / `delete`, destructors and `clear()` functions) — no `std::list`
or `std::stack`.

## Features

- Main menu: Play vs Computer, Multiplayer, Difficulty, Timer, Exit
- Full chess rules: castling, en passant, promotion, check, checkmate,
  stalemate, 50-move and insufficient-material draws, strict legal-move
  validation (you can never leave your own king in check)
- AI: Easy (random legal moves), Medium (material + piece-square evaluation),
  Hard (minimax with alpha-beta pruning, depth 3)
- Chess clock: 1 / 3 / 5 / 10 / 15 minutes, flag fall declares a winner,
  clock pauses when the game ends
- Move history panel (linked list), captured pieces, game status,
  check/checkmate messages, game-over screen
- Highlighting for the selected square, all legal destinations, the last move
  and a king in check
- UNDO (stack based; in computer mode it takes back both plies),
  NEW GAME with confirmation, MAIN MENU

## Controls

Click a piece to select it, click a highlighted square to move.
`U` = undo, `Esc` = back to the main menu.
