#include "Piece.h"

int Piece::value() const {
    switch (type) {
        case PieceType::Pawn:   return 100;
        case PieceType::Knight: return 320;
        case PieceType::Bishop: return 330;
        case PieceType::Rook:   return 500;
        case PieceType::Queen:  return 900;
        case PieceType::King:   return 20000;
        default:                return 0;
    }
}

char Piece::letter() const {
    char c = ' ';
    switch (type) {
        case PieceType::Pawn:   c = 'P'; break;
        case PieceType::Knight: c = 'N'; break;
        case PieceType::Bishop: c = 'B'; break;
        case PieceType::Rook:   c = 'R'; break;
        case PieceType::Queen:  c = 'Q'; break;
        case PieceType::King:   c = 'K'; break;
        default: return ' ';
    }
    if (color == Color::Black) c = static_cast<char>(c - 'A' + 'a');
    return c;
}

unsigned int Piece::glyph() const {
    // White glyphs start at U+2654, black glyphs at U+265A.
    unsigned int base = (color == Color::White) ? 0x2654u : 0x265Au;
    switch (type) {
        case PieceType::King:   return base + 0;
        case PieceType::Queen:  return base + 1;
        case PieceType::Rook:   return base + 2;
        case PieceType::Bishop: return base + 3;
        case PieceType::Knight: return base + 4;
        case PieceType::Pawn:   return base + 5;
        default:                return 0x20u;
    }
}

std::string pieceLetterSAN(PieceType t) {
    switch (t) {
        case PieceType::Knight: return "N";
        case PieceType::Bishop: return "B";
        case PieceType::Rook:   return "R";
        case PieceType::Queen:  return "Q";
        case PieceType::King:   return "K";
        default:                return "";
    }
}
