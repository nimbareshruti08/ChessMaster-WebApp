#pragma once
#include <string>

// ---------------------------------------------------------------------------
// Piece.h - basic chess piece representation
// ---------------------------------------------------------------------------

enum class PieceType { None = 0, Pawn, Knight, Bishop, Rook, Queen, King };

enum class Color { None = 0, White, Black };

inline Color opposite(Color c) {
    return c == Color::White ? Color::Black : (c == Color::Black ? Color::White : Color::None);
}

// A single square's content. Small value type stored inside the 2D board array.
struct Piece {
    PieceType type;
    Color color;

    Piece() : type(PieceType::None), color(Color::None) {}
    Piece(PieceType t, Color c) : type(t), color(c) {}

    bool isEmpty() const { return type == PieceType::None; }

    // Standard material values used by the AI evaluation function.
    int value() const;

    // 'P','N','B','R','Q','K' (uppercase = white, lowercase = black), ' ' if empty
    char letter() const;

    // Unicode chess glyph used by the GUI renderer.
    unsigned int glyph() const;
};

// Letter used in algebraic notation ("" for pawn).
std::string pieceLetterSAN(PieceType t);
