# Chess Master — C++ Web Application

A complete, fully playable, high-performance **Web Application** version of the **Chess Master** C++ desktop application.

This web application preserves **100% of the C++ logic, rule engine, algorithms, and required data structures** from the original C++ desktop project, while delivering a modern, glassmorphic Web interface.

---

## 📁 Repository Structure

```
ChessMaster-WebApp/
├── cpp/                        # Full original C++ Desktop Source Code
│   ├── include/                # Header files (Board.h, ChessAI.h, MoveLinkedList.h, MoveStack.h, etc.)
│   ├── src/                    # C++ source files (Board.cpp, ChessAI.cpp, MoveLinkedList.cpp, MoveStack.cpp, etc.)
│   ├── CMakeLists.txt          # CMake build configuration for Desktop SFML build
│   └── README.md
├── js/
│   ├── cpp_engine.js           # C++ Engine WebAssembly / JS logic layer (1-to-1 data structure translation)
│   └── app.js                  # Web UI Application Controller & Synthesized Sound System
├── index.html                  # Glassmorphic Web App Entrypoint (Semantic HTML5)
├── styles.css                  # Modern Glassmorphic CSS Design System with dark themes
├── package.json                # Project configuration & npm scripts
└── README.md                   # Project documentation
```

---

## 🛠️ Required Data Structures & Topics Preserved

All data structures from the original C++ project are strictly preserved line-for-line in the engine:

| Data Structure | C++ Header & Implementation | Used For |
|---|---|---|
| **2D Array** | `Board::squares[8][8]` (`Board.h` / `Board.cpp`) | The 8x8 chessboard matrix representation |
| **Linked List** | `MoveLinkedList` (`MoveLinkedList.h` / `MoveLinkedList.cpp`) | Dynamic move history log with `head`, `tail`, `MoveNode*` traversal, appending, and unlinking |
| **Stack** | `MoveStack` (`MoveStack.h` / `MoveStack.cpp`) | LIFO state snapshot stack with `top`, `StackNode*`, and `BoardState` for UNDO functionality |

---

## ✨ Features

- **Game Modes**: Play vs Computer (AI) & 2-Player Local Multiplayer.
- **AI Opponent**:
  - **Easy**: Random legal moves (with slight capture preference).
  - **Medium**: 1-ply greedy search + Piece-Square Tables (PST) evaluation.
  - **Hard**: Minimax search with **Alpha-Beta Pruning (Depth 3)** + PST evaluation + Move ordering cutoffs.
- **Chess Clock / Timers**: 1, 3, 5, 10, and 15-minute blitz/rapid countdown timers with flag fall detection.
- **Complete Chess Rules Engine**:
  - Legal move generation with King safety checks (cannot move into check).
  - Castling (Kingside `O-O` and Queenside `O-O-O`).
  - En Passant captures.
  - Pawn Promotion modal dialog (Queen, Rook, Bishop, Knight).
  - Check, Checkmate, Stalemate, 50-move rule draw, and Insufficient material draw.
  - Standard Algebraic Notation (SAN) disambiguation engine (e.g., `Nf3`, `Bxf7+`, `O-O`, `Qh5#`).
- **Interactive UI**:
  - Selected square glowing highlights.
  - Legal target indicators (dots for empty squares, ring indicators for captures).
  - Last move origin/destination highlights.
  - King in check crimson warning glow.
  - Dynamic board theme switcher (Classic Wood, Dark Slate, Emerald, Midnight).
  - Synthesized Web Audio sound effects for moves, captures, checks, and game over.
  - Full keyboard shortcuts: `U` (Undo), `N` (New Game), `Esc` (Main Menu).

---

## 🚀 Running the Web Application

### Option 1: Live Dev Server (Recommended)

1. Make sure Node.js is installed.
2. Run the development server in PowerShell / Terminal:
   ```bash
   npm run dev
   ```
3. Open the printed localhost URL (e.g. `http://localhost:5173`) in your web browser.

### Option 2: Direct Browser Entry

Simply open `index.html` directly in any modern Web Browser (Chrome, Firefox, Edge, Safari).

---

## ⚙️ Building Desktop C++ Application (Optional)

If you wish to compile the desktop C++ executable using CMake and SFML, navigate to the `cpp/` folder:

```bash
cd cpp
cmake -S . -B build
cmake --build build
./build/bin/ChessMaster
```
