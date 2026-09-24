#pragma once
#include <string>
#include <vector>
#include "Board.h"
#include "ChessAI.h"
#include "MoveLinkedList.h"
#include "MoveStack.h"
#include "Timer.h"

// ---------------------------------------------------------------------------
// ChessGame.h - game controller: rules + history + undo + clock + AI
// ---------------------------------------------------------------------------

enum class GameMode { Multiplayer = 0, VsComputer };

enum class GameResult {
    Ongoing = 0,
    WhiteWinsCheckmate,
    BlackWinsCheckmate,
    WhiteWinsTimeout,
    BlackWinsTimeout,
    Stalemate,
    Draw
};

class ChessGame {
public:
    ChessGame();

    // --- setup -----------------------------------------------------------
    void newGame();
    void setMode(GameMode m) { mode = m; }
    GameMode getMode() const { return mode; }
    void setDifficulty(Difficulty d) { ai.setDifficulty(d); }
    Difficulty getDifficulty() const { return ai.getDifficulty(); }
    void setMinutes(int m) { timer.setMinutes(m); }

    // --- play ------------------------------------------------------------
    // Legal destinations for the piece standing on (row,col).
    std::vector<Move> legalMovesFrom(int row, int col) const;

    // Plays a move if it is legal. Promotion defaults to Queen.
    bool makeMove(int fromRow, int fromCol, int toRow, int toCol,
                  PieceType promotion = PieceType::Queen);

    void playComputerMove();                 // computer plays for the side to move
    bool computerShouldMove() const;

    void update(float deltaSeconds);         // advances the clock
    bool undo();                             // STACK-powered undo

    // --- queries used by the GUI ----------------------------------------
    const Board& getBoard() const { return board; }
    Color turn() const { return board.sideToMove; }
    const Timer& getTimer() const { return timer; }
    GameResult result() const { return gameResult; }
    bool isOver() const { return gameResult != GameResult::Ongoing; }
    std::string statusText() const;
    bool inCheckNow() const { return board.inCheck(board.sideToMove); }

    const std::vector<Piece>& whiteCaptured() const { return capturedByWhite; }
    const std::vector<Piece>& blackCaptured() const { return capturedByBlack; }

    // LINKED LIST: move history exposed to the GUI panel.
    std::vector<Move> history() const { return moveHistory.traverseHistory(); }
    bool lastMove(Move& out) const { return moveHistory.getLastMove(out); }

    Color humanColor() const { return Color::White; } // human always plays white

private:
    Board board;                     // 2D ARRAY inside
    MoveLinkedList moveHistory;      // LINKED LIST: complete move history
    MoveStack undoStack;             // STACK: previous board states for UNDO
    Timer timer;
    ChessAI ai;

    GameMode mode;
    GameResult gameResult;

    std::vector<Piece> capturedByWhite;
    std::vector<Piece> capturedByBlack;

    BoardState snapshot() const;              // current state -> snapshot
    void restore(const BoardState& s);        // snapshot -> current state
    void checkGameOver();
};
