#include "MoveStack.h"

MoveStack::MoveStack() : top(nullptr), count(0) {}

MoveStack::~MoveStack() { clear(); }

// STACK: push the board state that existed BEFORE the move.
void MoveStack::push(const BoardState& state) {
    StackNode* node = new StackNode(state);
    node->next = top;
    top = node;
    ++count;
}

// STACK: pop the previous board state (UNDO).
bool MoveStack::pop(BoardState& out) {
    if (top == nullptr) return false;
    StackNode* node = top;
    out = node->state;
    top = node->next;
    delete node;
    --count;
    return true;
}

bool MoveStack::peek(BoardState& out) const {
    if (top == nullptr) return false;
    out = top->state;
    return true;
}

void MoveStack::clear() {
    while (top != nullptr) {
        StackNode* next = top->next;
        delete top;
        top = next;
    }
    count = 0;
}
