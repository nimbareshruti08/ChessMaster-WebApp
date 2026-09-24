#pragma once
#include <vector>
#include "Board.h"
#include "Piece.h"

// ---------------------------------------------------------------------------
// STACK: stores previous board states so that UNDO can restore them.
// Implemented from scratch with nodes (NOT std::vector / std::stack).
// ---------------------------------------------------------------------------

// A complete snapshot of the game taken before a move is executed.
struct BoardState {
    Board board;
    std::vector<Piece> whiteCaptured; // pieces captured BY white
    std::vector<Piece> blackCaptured; // pieces captured BY black
    float whiteTime = 0.f;            // remaining seconds
    float blackTime = 0.f;
};

struct StackNode {
    BoardState state;
    StackNode* next;
    explicit StackNode(const BoardState& s) : state(s), next(nullptr) {}
};

class MoveStack {
public:
    MoveStack();
    ~MoveStack();                       // frees every node

    void push(const BoardState& state); // save state before a move
    bool pop(BoardState& out);          // restore state on UNDO
    bool peek(BoardState& out) const;
    bool isEmpty() const { return top == nullptr; }
    int size() const { return count; }
    void clear();                       // delete all nodes

private:
    StackNode* top;
    int count;

    MoveStack(const MoveStack&);
    MoveStack& operator=(const MoveStack&);
};
