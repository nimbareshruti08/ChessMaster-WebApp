#pragma once
#include <vector>
#include <string>
#include "Move.h"

// ---------------------------------------------------------------------------
// LINKED LIST: stores the complete chess move history.
// Implemented from scratch with dynamically allocated nodes (no std::list).
// ---------------------------------------------------------------------------

struct MoveNode {
    Move move;
    MoveNode* next;
    explicit MoveNode(const Move& m) : move(m), next(nullptr) {}
};

class MoveLinkedList {
public:
    MoveLinkedList();
    ~MoveLinkedList();                       // frees every node

    void addMove(const Move& m);             // append at the tail
    bool removeLastMove();                   // used by UNDO
    bool getLastMove(Move& out) const;       // most recent move
    void clearHistory();                     // delete all nodes
    void displayHistory() const;             // prints the history to stdout
    std::vector<Move> traverseHistory() const; // walks the list front -> back
    int size() const { return count; }
    bool empty() const { return head == nullptr; }

private:
    MoveNode* head;
    MoveNode* tail;
    int count;

    MoveLinkedList(const MoveLinkedList&);            // non-copyable
    MoveLinkedList& operator=(const MoveLinkedList&);
};
