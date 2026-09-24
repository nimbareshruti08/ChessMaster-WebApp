#include "ChessAI.h"
#include <algorithm>
#include <cstdlib>
#include <limits>

// ---------------------------------------------------------------------------
// Piece-square tables (white's point of view, row 0 == rank 8)
// They give the engine a basic sense of good squares.
// ---------------------------------------------------------------------------
static const int PAWN_PST[8][8] = {
    { 0,  0,  0,  0,  0,  0,  0,  0},
    {50, 50, 50, 50, 50, 50, 50, 50},
    {10, 10, 20, 30, 30, 20, 10, 10},
    { 5,  5, 10, 25, 25, 10,  5,  5},
    { 0,  0,  0, 20, 20,  0,  0,  0},
    { 5, -5,-10,  0,  0,-10, -5,  5},
    { 5, 10, 10,-20,-20, 10, 10,  5},
    { 0,  0,  0,  0,  0,  0,  0,  0}
};
static const int KNIGHT_PST[8][8] = {
    {-50,-40,-30,-30,-30,-30,-40,-50},
    {-40,-20,  0,  0,  0,  0,-20,-40},
    {-30,  0, 10, 15, 15, 10,  0,-30},
    {-30,  5, 15, 20, 20, 15,  5,-30},
    {-30,  0, 15, 20, 20, 15,  0,-30},
    {-30,  5, 10, 15, 15, 10,  5,-30},
    {-40,-20,  0,  5,  5,  0,-20,-40},
    {-50,-40,-30,-30,-30,-30,-40,-50}
};
static const int BISHOP_PST[8][8] = {
    {-20,-10,-10,-10,-10,-10,-10,-20},
    {-10,  0,  0,  0,  0,  0,  0,-10},
    {-10,  0,  5, 10, 10,  5,  0,-10},
    {-10,  5,  5, 10, 10,  5,  5,-10},
    {-10,  0, 10, 10, 10, 10,  0,-10},
    {-10, 10, 10, 10, 10, 10, 10,-10},
    {-10,  5,  0,  0,  0,  0,  5,-10},
    {-20,-10,-10,-10,-10,-10,-10,-20}
};
static const int ROOK_PST[8][8] = {
    {  0,  0,  0,  0,  0,  0,  0,  0},
    {  5, 10, 10, 10, 10, 10, 10,  5},
    { -5,  0,  0,  0,  0,  0,  0, -5},
    { -5,  0,  0,  0,  0,  0,  0, -5},
    { -5,  0,  0,  0,  0,  0,  0, -5},
    { -5,  0,  0,  0,  0,  0,  0, -5},
    { -5,  0,  0,  0,  0,  0,  0, -5},
    {  0,  0,  0,  5,  5,  0,  0,  0}
};
static const int QUEEN_PST[8][8] = {
    {-20,-10,-10, -5, -5,-10,-10,-20},
    {-10,  0,  0,  0,  0,  0,  0,-10},
    {-10,  0,  5,  5,  5,  5,  0,-10},
    { -5,  0,  5,  5,  5,  5,  0, -5},
    {  0,  0,  5,  5,  5,  5,  0, -5},
    {-10,  5,  5,  5,  5,  5,  0,-10},
    {-10,  0,  5,  0,  0,  0,  0,-10},
    {-20,-10,-10, -5, -5,-10,-10,-20}
};
static const int KING_PST[8][8] = {
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-20,-30,-30,-40,-40,-30,-30,-20},
    {-10,-20,-20,-20,-20,-20,-20,-10},
    { 20, 20,  0,  0,  0, 20, 20, 20},
    { 20, 30, 10,  0,  0, 10, 30, 20}
};

static int pstValue(PieceType t, Color color, int r, int c) {
    int row = (color == Color::White) ? r : 7 - r; // mirror for black
    switch (t) {
        case PieceType::Pawn:   return PAWN_PST[row][c];
        case PieceType::Knight: return KNIGHT_PST[row][c];
        case PieceType::Bishop: return BISHOP_PST[row][c];
        case PieceType::Rook:   return ROOK_PST[row][c];
        case PieceType::Queen:  return QUEEN_PST[row][c];
        case PieceType::King:   return KING_PST[row][c];
        default:                return 0;
    }
}

ChessAI::ChessAI(Difficulty d) : difficulty(d) {}

// Material + piece-square evaluation, positive = good for White.
int ChessAI::evaluate(const Board& board) {
    int score = 0;
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            const Piece& p = board.at(r, c);
            if (p.isEmpty()) continue;
            int v = p.value() + pstValue(p.type, p.color, r, c);
            score += (p.color == Color::White) ? v : -v;
        }
    }
    return score;
}

Move ChessAI::chooseMove(const Board& board) {
    switch (difficulty) {
        case Difficulty::Easy:   return chooseRandomMove(board);
        case Difficulty::Medium: return chooseGreedyMove(board);
        default:                 return chooseSearchMove(board);
    }
}

// EASY: pick any legal move at random (slight preference for captures).
Move ChessAI::chooseRandomMove(const Board& board) {
    std::vector<Move> moves;
    board.generateLegalMoves(moves);
    if (moves.empty()) return Move();

    std::vector<Move> captures;
    for (size_t i = 0; i < moves.size(); ++i)
        if (!moves[i].captured.isEmpty()) captures.push_back(moves[i]);

    if (!captures.empty() && (std::rand() % 100) < 40)
        return captures[std::rand() % captures.size()];

    return moves[std::rand() % moves.size()];
}

// MEDIUM: evaluate every legal move one ply deep and keep the best one.
Move ChessAI::chooseGreedyMove(const Board& board) {
    std::vector<Move> moves;
    board.generateLegalMoves(moves);
    if (moves.empty()) return Move();

    bool maximizing = (board.sideToMove == Color::White);
    int bestScore = maximizing ? -1000000 : 1000000;
    std::vector<Move> best;

    for (size_t i = 0; i < moves.size(); ++i) {
        Board copy = board;
        copy.applyMove(moves[i]);

        int score = evaluate(copy);
        if (copy.isCheckmate()) score += maximizing ? 100000 : -100000;

        if ((maximizing && score > bestScore) || (!maximizing && score < bestScore)) {
            bestScore = score;
            best.clear();
            best.push_back(moves[i]);
        } else if (score == bestScore) {
            best.push_back(moves[i]);
        }
    }
    return best[std::rand() % best.size()];
}

// HARD: minimax search with alpha-beta pruning.
int ChessAI::minimax(Board board, int depth, int alpha, int beta, bool maximizing) {
    std::vector<Move> moves;
    board.generateLegalMoves(moves);

    if (moves.empty()) {
        if (board.inCheck(board.sideToMove))
            return maximizing ? -900000 - depth : 900000 + depth; // checkmate
        return 0;                                                 // stalemate
    }
    if (depth == 0) return evaluate(board);

    // Simple move ordering: captures first (improves alpha-beta cutoffs).
    std::stable_sort(moves.begin(), moves.end(), [](const Move& a, const Move& b) {
        return a.captured.value() > b.captured.value();
    });

    if (maximizing) {
        int best = -1000000;
        for (size_t i = 0; i < moves.size(); ++i) {
            Board copy = board;
            copy.applyMove(moves[i]);
            best = std::max(best, minimax(copy, depth - 1, alpha, beta, false));
            alpha = std::max(alpha, best);
            if (beta <= alpha) break; // alpha-beta pruning
        }
        return best;
    }

    int best = 1000000;
    for (size_t i = 0; i < moves.size(); ++i) {
        Board copy = board;
        copy.applyMove(moves[i]);
        best = std::min(best, minimax(copy, depth - 1, alpha, beta, true));
        beta = std::min(beta, best);
        if (beta <= alpha) break;     // alpha-beta pruning
    }
    return best;
}

Move ChessAI::chooseSearchMove(const Board& board) {
    std::vector<Move> moves;
    board.generateLegalMoves(moves);
    if (moves.empty()) return Move();

    const int depth = 3; // keeps the application responsive
    bool maximizing = (board.sideToMove == Color::White);
    int bestScore = maximizing ? -1000000 : 1000000;
    Move bestMove = moves[0];

    std::stable_sort(moves.begin(), moves.end(), [](const Move& a, const Move& b) {
        return a.captured.value() > b.captured.value();
    });

    for (size_t i = 0; i < moves.size(); ++i) {
        Board copy = board;
        copy.applyMove(moves[i]);
        int score = minimax(copy, depth - 1, -1000000, 1000000, !maximizing);
        if ((maximizing && score > bestScore) || (!maximizing && score < bestScore)) {
            bestScore = score;
            bestMove = moves[i];
        }
    }
    return bestMove;
}
