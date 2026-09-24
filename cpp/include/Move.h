#pragma once
#include <string>
#include "Piece.h"

// ---------------------------------------------------------------------------
// Move.h - one chess move (also stored in the LINKED LIST of move history)
// ---------------------------------------------------------------------------

struct Move {
    int fromRow = -1, fromCol = -1;
    int toRow = -1, toCol = -1;

    PieceType promotion = PieceType::None; // piece a pawn promotes to
    Piece moved;                           // piece that moved
    Piece captured;                        // captured piece (empty if none)

    bool isCastleKingSide = false;
    bool isCastleQueenSide = false;
    bool isEnPassant = false;

    std::string notation; // SAN text such as "Nf3", "exd5", "O-O", "Qh5#"

    Move() {}
    Move(int fr, int fc, int tr, int tc) : fromRow(fr), fromCol(fc), toRow(tr), toCol(tc) {}

    bool isValid() const { return fromRow >= 0 && toRow >= 0; }

    bool sameSquares(const Move& o) const {
        return fromRow == o.fromRow && fromCol == o.fromCol &&
               toRow == o.toRow && toCol == o.toCol;
    }

    // "e2" style coordinate of a board square (row 0 == rank 8).
    static std::string squareName(int row, int col);
};
