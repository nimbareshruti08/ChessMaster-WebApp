#include "MoveLinkedList.h"
#include <iostream>

MoveLinkedList::MoveLinkedList() : head(nullptr), tail(nullptr), count(0) {}

MoveLinkedList::~MoveLinkedList() { clearHistory(); }

// LINKED LIST: every valid move is appended here.
void MoveLinkedList::addMove(const Move& m) {
    MoveNode* node = new MoveNode(m);
    if (head == nullptr) {
        head = tail = node;
    } else {
        tail->next = node;
        tail = node;
    }
    ++count;
}

// Removes the tail node (needed when a move is undone).
bool MoveLinkedList::removeLastMove() {
    if (head == nullptr) return false;

    if (head == tail) {                 // single element
        delete head;
        head = tail = nullptr;
        count = 0;
        return true;
    }

    MoveNode* cur = head;
    while (cur->next != tail) cur = cur->next;   // traverse to the node before tail
    delete tail;
    tail = cur;
    tail->next = nullptr;
    --count;
    return true;
}

bool MoveLinkedList::getLastMove(Move& out) const {
    if (tail == nullptr) return false;
    out = tail->move;
    return true;
}

void MoveLinkedList::clearHistory() {
    MoveNode* cur = head;
    while (cur != nullptr) {
        MoveNode* next = cur->next;
        delete cur;                     // proper node deletion -> no leaks
        cur = next;
    }
    head = tail = nullptr;
    count = 0;
}

std::vector<Move> MoveLinkedList::traverseHistory() const {
    std::vector<Move> result;
    for (MoveNode* cur = head; cur != nullptr; cur = cur->next)
        result.push_back(cur->move);
    return result;
}

void MoveLinkedList::displayHistory() const {
    int i = 0;
    for (MoveNode* cur = head; cur != nullptr; cur = cur->next, ++i) {
        if (i % 2 == 0) std::cout << (i / 2 + 1) << ". ";
        std::cout << cur->move.notation << (i % 2 == 0 ? " " : "\n");
    }
    std::cout << std::endl;
}
