#include "Move.h"

std::string Move::squareName(int row, int col) {
    if (row < 0 || row > 7 || col < 0 || col > 7) return "??";
    std::string s;
    s += static_cast<char>('a' + col);
    s += static_cast<char>('8' - row);
    return s;
}
