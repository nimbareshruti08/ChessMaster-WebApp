#include "Board.h"
#include <cstdlib>

// ---------------------------------------------------------------------------
// Direction tables
// ---------------------------------------------------------------------------
static const int KNIGHT_DIRS[8][2] = {{-2,-1},{-2,1},{-1,-2},{-1,2},{1,-2},{1,2},{2,-1},{2,1}};
static const int KING_DIRS[8][2]   = {{-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1}};
static const int ROOK_DIRS[4][2]   = {{-1,0},{1,0},{0,-1},{0,1}};
static const int BISHOP_DIRS[4][2] = {{-1,-1},{-1,1},{1,-1},{1,1}};

Board::Board() { reset(); }

void Board::reset() {
    for (int r = 0; r < 8; ++r)
        for (int c = 0; c < 8; ++c)
            squares[r][c] = Piece();

    const PieceType backRank[8] = {PieceType::Rook, PieceType::Knight, PieceType::Bishop,
                                   PieceType::Queen, PieceType::King, PieceType::Bishop,
                                   PieceType::Knight, PieceType::Rook};

    for (int c = 0; c < 8; ++c) {
        squares[0][c] = Piece(backRank[c], Color::Black);
        squares[1][c] = Piece(PieceType::Pawn, Color::Black);
        squares[6][c] = Piece(PieceType::Pawn, Color::White);
        squares[7][c] = Piece(backRank[c], Color::White);
    }

    sideToMove = Color::White;
    whiteCanCastleKing = whiteCanCastleQueen = true;
    blackCanCastleKing = blackCanCastleQueen = true;
    epRow = epCol = -1;
    halfmoveClock = 0;
    fullmoveNumber = 1;
}

// ---------------------------------------------------------------------------
// Move generation helpers
// ---------------------------------------------------------------------------
void Board::addSlidingMoves(int r, int c, Color side, const int dirs[][2], int count,
                            std::vector<Move>& out) const {
    for (int d = 0; d < count; ++d) {
        int nr = r + dirs[d][0], nc = c + dirs[d][1];
        while (inside(nr, nc)) {
            const Piece& target = squares[nr][nc];
            if (target.isEmpty()) {
                Move m(r, c, nr, nc);
                m.moved = squares[r][c];
                out.push_back(m);
            } else {
                if (target.color != side) {
                    Move m(r, c, nr, nc);
                    m.moved = squares[r][c];
                    m.captured = target;
                    out.push_back(m);
                }
                break; // blocked
            }
            nr += dirs[d][0];
            nc += dirs[d][1];
        }
    }
}

void Board::addStepMoves(int r, int c, Color side, const int dirs[][2], int count,
                         std::vector<Move>& out) const {
    for (int d = 0; d < count; ++d) {
        int nr = r + dirs[d][0], nc = c + dirs[d][1];
        if (!inside(nr, nc)) continue;
        const Piece& target = squares[nr][nc];
        if (!target.isEmpty() && target.color == side) continue;
        Move m(r, c, nr, nc);
        m.moved = squares[r][c];
        m.captured = target;
        out.push_back(m);
    }
}

void Board::addPawnMoves(int r, int c, Color side, std::vector<Move>& out) const {
    int dir = (side == Color::White) ? -1 : 1;      // white moves up the array
    int startRow = (side == Color::White) ? 6 : 1;
    int promoRow = (side == Color::White) ? 0 : 7;

    auto pushMaybePromo = [&](Move m) {
        m.moved = squares[r][c];
        if (m.toRow == promoRow) {
            const PieceType promos[4] = {PieceType::Queen, PieceType::Rook,
                                         PieceType::Bishop, PieceType::Knight};
            for (int i = 0; i < 4; ++i) { m.promotion = promos[i]; out.push_back(m); }
        } else {
            out.push_back(m);
        }
    };

    // single / double forward push
    int nr = r + dir;
    if (inside(nr, c) && squares[nr][c].isEmpty()) {
        pushMaybePromo(Move(r, c, nr, c));
        int nr2 = r + 2 * dir;
        if (r == startRow && inside(nr2, c) && squares[nr2][c].isEmpty()) {
            Move m(r, c, nr2, c);
            m.moved = squares[r][c];
            out.push_back(m);
        }
    }

    // diagonal captures (+ en passant)
    for (int dc = -1; dc <= 1; dc += 2) {
        int ncol = c + dc;
        if (!inside(nr, ncol)) continue;
        const Piece& target = squares[nr][ncol];
        if (!target.isEmpty() && target.color != side) {
            Move m(r, c, nr, ncol);
            m.captured = target;
            pushMaybePromo(m);
        } else if (target.isEmpty() && nr == epRow && ncol == epCol) {
            Move m(r, c, nr, ncol);
            m.moved = squares[r][c];
            m.isEnPassant = true;
            m.captured = Piece(PieceType::Pawn, opposite(side));
            out.push_back(m);
        }
    }
}

void Board::addCastlingMoves(Color side, std::vector<Move>& out) const {
    int row = (side == Color::White) ? 7 : 0;
    bool canKing  = (side == Color::White) ? whiteCanCastleKing  : blackCanCastleKing;
    bool canQueen = (side == Color::White) ? whiteCanCastleQueen : blackCanCastleQueen;

    const Piece& king = squares[row][4];
    if (king.type != PieceType::King || king.color != side) return;
    if (inCheck(side)) return; // cannot castle out of check

    Color enemy = opposite(side);

    if (canKing && squares[row][5].isEmpty() && squares[row][6].isEmpty() &&
        squares[row][7].type == PieceType::Rook && squares[row][7].color == side &&
        !isSquareAttacked(row, 5, enemy) && !isSquareAttacked(row, 6, enemy)) {
        Move m(row, 4, row, 6);
        m.moved = king;
        m.isCastleKingSide = true;
        out.push_back(m);
    }

    if (canQueen && squares[row][3].isEmpty() && squares[row][2].isEmpty() &&
        squares[row][1].isEmpty() &&
        squares[row][0].type == PieceType::Rook && squares[row][0].color == side &&
        !isSquareAttacked(row, 3, enemy) && !isSquareAttacked(row, 2, enemy)) {
        Move m(row, 4, row, 2);
        m.moved = king;
        m.isCastleQueenSide = true;
        out.push_back(m);
    }
}

void Board::generatePseudoMoves(Color side, std::vector<Move>& out) const {
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            const Piece& p = squares[r][c];
            if (p.isEmpty() || p.color != side) continue;
            switch (p.type) {
                case PieceType::Pawn:   addPawnMoves(r, c, side, out); break;
                case PieceType::Knight: addStepMoves(r, c, side, KNIGHT_DIRS, 8, out); break;
                case PieceType::King:   addStepMoves(r, c, side, KING_DIRS, 8, out); break;
                case PieceType::Bishop: addSlidingMoves(r, c, side, BISHOP_DIRS, 4, out); break;
                case PieceType::Rook:   addSlidingMoves(r, c, side, ROOK_DIRS, 4, out); break;
                case PieceType::Queen:
                    addSlidingMoves(r, c, side, ROOK_DIRS, 4, out);
                    addSlidingMoves(r, c, side, BISHOP_DIRS, 4, out);
                    break;
                default: break;
            }
        }
    }
    addCastlingMoves(side, out);
}

// Legal moves = pseudo-legal moves that do not leave our own king in check.
void Board::generateLegalMovesFor(Color side, std::vector<Move>& out) const {
    std::vector<Move> pseudo;
    pseudo.reserve(64);
    generatePseudoMoves(side, pseudo);

    for (size_t i = 0; i < pseudo.size(); ++i) {
        Board copy = *this;          // KING SAFETY: try the move on a copy
        copy.applyMove(pseudo[i]);
        if (!copy.inCheck(side)) out.push_back(pseudo[i]);
    }
}

void Board::generateLegalMoves(std::vector<Move>& out) const {
    generateLegalMovesFor(sideToMove, out);
}

// ---------------------------------------------------------------------------
// Attack detection
// ---------------------------------------------------------------------------
bool Board::isSquareAttacked(int r, int c, Color by) const {
    // pawns
    int dir = (by == Color::White) ? 1 : -1; // square is "below" an attacking white pawn
    for (int dc = -1; dc <= 1; dc += 2) {
        int pr = r + dir, pc = c + dc;
        if (inside(pr, pc)) {
            const Piece& p = squares[pr][pc];
            if (p.type == PieceType::Pawn && p.color == by) return true;
        }
    }
    // knights
    for (int d = 0; d < 8; ++d) {
        int nr = r + KNIGHT_DIRS[d][0], nc = c + KNIGHT_DIRS[d][1];
        if (!inside(nr, nc)) continue;
        const Piece& p = squares[nr][nc];
        if (p.type == PieceType::Knight && p.color == by) return true;
    }
    // king
    for (int d = 0; d < 8; ++d) {
        int nr = r + KING_DIRS[d][0], nc = c + KING_DIRS[d][1];
        if (!inside(nr, nc)) continue;
        const Piece& p = squares[nr][nc];
        if (p.type == PieceType::King && p.color == by) return true;
    }
    // rook / queen lines
    for (int d = 0; d < 4; ++d) {
        int nr = r + ROOK_DIRS[d][0], nc = c + ROOK_DIRS[d][1];
        while (inside(nr, nc)) {
            const Piece& p = squares[nr][nc];
            if (!p.isEmpty()) {
                if (p.color == by && (p.type == PieceType::Rook || p.type == PieceType::Queen))
                    return true;
                break;
            }
            nr += ROOK_DIRS[d][0];
            nc += ROOK_DIRS[d][1];
        }
    }
    // bishop / queen diagonals
    for (int d = 0; d < 4; ++d) {
        int nr = r + BISHOP_DIRS[d][0], nc = c + BISHOP_DIRS[d][1];
        while (inside(nr, nc)) {
            const Piece& p = squares[nr][nc];
            if (!p.isEmpty()) {
                if (p.color == by && (p.type == PieceType::Bishop || p.type == PieceType::Queen))
                    return true;
                break;
            }
            nr += BISHOP_DIRS[d][0];
            nc += BISHOP_DIRS[d][1];
        }
    }
    return false;
}

bool Board::findKing(Color c, int& row, int& col) const {
    for (int r = 0; r < 8; ++r)
        for (int cc = 0; cc < 8; ++cc)
            if (squares[r][cc].type == PieceType::King && squares[r][cc].color == c) {
                row = r; col = cc; return true;
            }
    return false;
}

bool Board::inCheck(Color c) const {
    int kr, kc;
    if (!findKing(c, kr, kc)) return false;
    return isSquareAttacked(kr, kc, opposite(c));
}

bool Board::isCheckmate() const {
    if (!inCheck(sideToMove)) return false;
    std::vector<Move> moves;
    generateLegalMoves(moves);
    return moves.empty();
}

bool Board::isStalemate() const {
    if (inCheck(sideToMove)) return false;
    std::vector<Move> moves;
    generateLegalMoves(moves);
    return moves.empty();
}

bool Board::insufficientMaterial() const {
    int minor = 0;
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            const Piece& p = squares[r][c];
            if (p.isEmpty() || p.type == PieceType::King) continue;
            if (p.type == PieceType::Bishop || p.type == PieceType::Knight) { ++minor; continue; }
            return false; // pawn, rook or queen still on the board
        }
    }
    return minor <= 1;
}

// ---------------------------------------------------------------------------
// Applying a move
// ---------------------------------------------------------------------------
void Board::applyMove(const Move& m) {
    Piece moving = squares[m.fromRow][m.fromCol];

    bool capture = !squares[m.toRow][m.toCol].isEmpty() || m.isEnPassant;

    // en passant: remove the pawn that is beside the destination square
    if (m.isEnPassant) {
        int capturedRow = (moving.color == Color::White) ? m.toRow + 1 : m.toRow - 1;
        squares[capturedRow][m.toCol] = Piece();
    }

    squares[m.toRow][m.toCol] = moving;
    squares[m.fromRow][m.fromCol] = Piece();

    // promotion
    if (m.promotion != PieceType::None)
        squares[m.toRow][m.toCol] = Piece(m.promotion, moving.color);

    // castling: move the rook as well
    if (m.isCastleKingSide) {
        squares[m.toRow][5] = squares[m.toRow][7];
        squares[m.toRow][7] = Piece();
    } else if (m.isCastleQueenSide) {
        squares[m.toRow][3] = squares[m.toRow][0];
        squares[m.toRow][0] = Piece();
    }

    // update castling rights
    if (moving.type == PieceType::King) {
        if (moving.color == Color::White) whiteCanCastleKing = whiteCanCastleQueen = false;
        else                              blackCanCastleKing = blackCanCastleQueen = false;
    }
    if (m.fromRow == 7 && m.fromCol == 0) whiteCanCastleQueen = false;
    if (m.fromRow == 7 && m.fromCol == 7) whiteCanCastleKing  = false;
    if (m.fromRow == 0 && m.fromCol == 0) blackCanCastleQueen = false;
    if (m.fromRow == 0 && m.fromCol == 7) blackCanCastleKing  = false;
    if (m.toRow == 7 && m.toCol == 0) whiteCanCastleQueen = false;
    if (m.toRow == 7 && m.toCol == 7) whiteCanCastleKing  = false;
    if (m.toRow == 0 && m.toCol == 0) blackCanCastleQueen = false;
    if (m.toRow == 0 && m.toCol == 7) blackCanCastleKing  = false;

    // en passant target for the next move
    epRow = epCol = -1;
    if (moving.type == PieceType::Pawn && std::abs(m.toRow - m.fromRow) == 2) {
        epRow = (m.fromRow + m.toRow) / 2;
        epCol = m.fromCol;
    }

    if (moving.type == PieceType::Pawn || capture) halfmoveClock = 0;
    else ++halfmoveClock;

    if (sideToMove == Color::Black) ++fullmoveNumber;
    sideToMove = opposite(sideToMove);
}

// ---------------------------------------------------------------------------
// Standard Algebraic Notation
// ---------------------------------------------------------------------------
std::string Board::moveToSAN(const Move& m) const {
    if (m.isCastleKingSide || m.isCastleQueenSide) {
        std::string s = m.isCastleKingSide ? "O-O" : "O-O-O";
        Board copy = *this;
        copy.applyMove(m);
        if (copy.isCheckmate()) s += "#";
        else if (copy.inCheck(copy.sideToMove)) s += "+";
        return s;
    }

    Piece moving = squares[m.fromRow][m.fromCol];
    bool capture = !squares[m.toRow][m.toCol].isEmpty() || m.isEnPassant;
    std::string s;

    if (moving.type == PieceType::Pawn) {
        if (capture) { s += static_cast<char>('a' + m.fromCol); s += "x"; }
    } else {
        s += pieceLetterSAN(moving.type);

        // disambiguation: another same-type piece able to reach the same square
        std::vector<Move> legal;
        generateLegalMovesFor(moving.color, legal);
        bool needFile = false, needRank = false, ambiguous = false;
        for (size_t i = 0; i < legal.size(); ++i) {
            const Move& o = legal[i];
            if (o.toRow != m.toRow || o.toCol != m.toCol) continue;
            if (o.fromRow == m.fromRow && o.fromCol == m.fromCol) continue;
            if (squares[o.fromRow][o.fromCol].type != moving.type) continue;
            ambiguous = true;
            if (o.fromCol == m.fromCol) needRank = true; else needFile = true;
        }
        if (ambiguous) {
            if (needFile || !needRank) s += static_cast<char>('a' + m.fromCol);
            if (needRank)              s += static_cast<char>('8' - m.fromRow);
        }
        if (capture) s += "x";
    }

    s += Move::squareName(m.toRow, m.toCol);
    if (m.promotion != PieceType::None) s += "=" + pieceLetterSAN(m.promotion);

    Board copy = *this;
    copy.applyMove(m);
    if (copy.isCheckmate()) s += "#";
    else if (copy.inCheck(copy.sideToMove)) s += "+";
    return s;
}
