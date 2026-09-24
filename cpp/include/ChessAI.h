#pragma once
#include <vector>
#include "Board.h"
#include "Move.h"

// ---------------------------------------------------------------------------
// ChessAI.h - computer opponent (Easy / Medium / Hard)
// ---------------------------------------------------------------------------

enum class Difficulty { Easy = 0, Medium, Hard };

class ChessAI {
public:
    explicit ChessAI(Difficulty d = Difficulty::Medium);

    void setDifficulty(Difficulty d) { difficulty = d; }
    Difficulty getDifficulty() const { return difficulty; }

    // Returns the chosen legal move. move.isValid() == false when none exists.
    Move chooseMove(const Board& board);

    // Static evaluation from White's point of view (centipawns).
    static int evaluate(const Board& board);

private:
    Difficulty difficulty;

    Move chooseRandomMove(const Board& board);      // Easy
    Move chooseGreedyMove(const Board& board);      // Medium (1-ply + captures)
    Move chooseSearchMove(const Board& board);      // Hard  (minimax + alpha-beta)

    // Minimax with alpha-beta pruning.
    static int minimax(Board board, int depth, int alpha, int beta, bool maximizing);
};
