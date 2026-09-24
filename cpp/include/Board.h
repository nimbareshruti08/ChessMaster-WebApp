#pragma once
#include <vector>
#include <string>
#include "Piece.h"
#include "Move.h"

// ---------------------------------------------------------------------------
// Board.h - chess position + full rule engine
//
// 2D ARRAY: the chessboard itself is stored in `squares[8][8]`.
// Row 0 is rank 8 (black's back rank), row 7 is rank 1 (white's back rank).
// Column 0 is file 'a'.
// ---------------------------------------------------------------------------

class Board {
public:
    Board();

    // 2D ARRAY: the chessboard representation required by the project.
    Piece squares[8][8];

    Color sideToMove;

    // Castling rights
    bool whiteCanCastleKing, whiteCanCastleQueen;
    bool blackCanCastleKing, blackCanCastleQueen;

    // En-passant target square (-1 when unavailable)
    int epRow, epCol;

    int halfmoveClock;   // for the 50-move rule
    int fullmoveNumber;

    void reset();                       // set up the standard start position
    static bool inside(int r, int c) { return r >= 0 && r < 8 && c >= 0 && c < 8; }
    const Piece& at(int r, int c) const { return squares[r][c]; }

    // --- move generation -------------------------------------------------
    void generatePseudoMoves(Color side, std::vector<Move>& out) const;
    void generateLegalMoves(std::vector<Move>& out) const;               // side to move
    void generateLegalMovesFor(Color side, std::vector<Move>& out) const;

    // --- rules / state ---------------------------------------------------
    bool isSquareAttacked(int r, int c, Color by) const;
    bool findKing(Color c, int& row, int& col) const;
    bool inCheck(Color c) const;
    bool isCheckmate() const;
    bool isStalemate() const;
    bool insufficientMaterial() const;

    // Applies a (legal) move and switches the side to move.
    void applyMove(const Move& m);

    // Standard algebraic notation for a move in THIS position (before applying).
    std::string moveToSAN(const Move& m) const;

private:
    void addPawnMoves(int r, int c, Color side, std::vector<Move>& out) const;
    void addSlidingMoves(int r, int c, Color side, const int dirs[][2], int count,
                         std::vector<Move>& out) const;
    void addStepMoves(int r, int c, Color side, const int dirs[][2], int count,
                      std::vector<Move>& out) const;
    void addCastlingMoves(Color side, std::vector<Move>& out) const;
};
