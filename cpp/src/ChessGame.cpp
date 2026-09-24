#include "ChessGame.h"

ChessGame::ChessGame()
    : timer(10), ai(Difficulty::Medium), mode(GameMode::Multiplayer),
      gameResult(GameResult::Ongoing) {
    newGame();
}

// ---------------------------------------------------------------------------
// New game: reset EVERYTHING (board, clocks, linked list, stack, captures)
// ---------------------------------------------------------------------------
void ChessGame::newGame() {
    board.reset();
    moveHistory.clearHistory();   // LINKED LIST cleared
    undoStack.clear();            // STACK cleared
    capturedByWhite.clear();
    capturedByBlack.clear();
    timer.reset();
    gameResult = GameResult::Ongoing;
}

BoardState ChessGame::snapshot() const {
    BoardState s;
    s.board = board;
    s.whiteCaptured = capturedByWhite;
    s.blackCaptured = capturedByBlack;
    s.whiteTime = timer.whiteTime();
    s.blackTime = timer.blackTime();
    return s;
}

void ChessGame::restore(const BoardState& s) {
    board = s.board;
    capturedByWhite = s.whiteCaptured;
    capturedByBlack = s.blackCaptured;
    timer.setTimes(s.whiteTime, s.blackTime);
}

std::vector<Move> ChessGame::legalMovesFrom(int row, int col) const {
    std::vector<Move> all, result;
    board.generateLegalMoves(all);
    for (size_t i = 0; i < all.size(); ++i)
        if (all[i].fromRow == row && all[i].fromCol == col) result.push_back(all[i]);
    return result;
}

bool ChessGame::makeMove(int fromRow, int fromCol, int toRow, int toCol, PieceType promotion) {
    if (isOver()) return false;

    std::vector<Move> legal;
    board.generateLegalMoves(legal);

    Move chosen;
    bool found = false;
    for (size_t i = 0; i < legal.size(); ++i) {
        const Move& m = legal[i];
        if (m.fromRow == fromRow && m.fromCol == fromCol &&
            m.toRow == toRow && m.toCol == toCol) {
            // pick the requested promotion piece when several variants exist
            if (m.promotion == PieceType::None || m.promotion == promotion) {
                chosen = m;
                found = true;
                break;
            }
        }
    }
    if (!found) return false;   // ILLEGAL MOVE: silently rejected

    // STACK: save the state that existed BEFORE the move, then execute it.
    undoStack.push(snapshot());

    chosen.notation = board.moveToSAN(chosen);
    chosen.moved = board.at(fromRow, fromCol);

    if (!chosen.captured.isEmpty()) {
        if (chosen.moved.color == Color::White) capturedByWhite.push_back(chosen.captured);
        else                                    capturedByBlack.push_back(chosen.captured);
    }

    board.applyMove(chosen);

    // LINKED LIST: append the move to the history.
    moveHistory.addMove(chosen);

    checkGameOver();
    return true;
}

bool ChessGame::computerShouldMove() const {
    return mode == GameMode::VsComputer && !isOver() && board.sideToMove != humanColor();
}

void ChessGame::playComputerMove() {
    if (!computerShouldMove()) return;
    Move m = ai.chooseMove(board);
    if (!m.isValid()) { checkGameOver(); return; }
    makeMove(m.fromRow, m.fromCol, m.toRow, m.toCol,
             m.promotion == PieceType::None ? PieceType::Queen : m.promotion);
}

void ChessGame::update(float deltaSeconds) {
    if (isOver()) return;                        // the clock pauses when the game ends
    if (moveHistory.size() == 0) return;         // clock starts on the first move

    timer.tick(board.sideToMove, deltaSeconds);

    if (timer.whiteFlagged())      gameResult = GameResult::BlackWinsTimeout;
    else if (timer.blackFlagged()) gameResult = GameResult::WhiteWinsTimeout;
}

// ---------------------------------------------------------------------------
// UNDO - pops the previous board state from the STACK
// ---------------------------------------------------------------------------
bool ChessGame::undo() {
    BoardState state;
    if (!undoStack.pop(state)) return false;

    restore(state);
    moveHistory.removeLastMove();   // keep the LINKED LIST in sync
    gameResult = GameResult::Ongoing;

    // Against the computer: also undo the player's own move so the human
    // gets his turn back.
    if (mode == GameMode::VsComputer && board.sideToMove != humanColor()) {
        if (undoStack.pop(state)) {
            restore(state);
            moveHistory.removeLastMove();
        }
    }
    return true;
}

void ChessGame::checkGameOver() {
    if (board.isCheckmate()) {
        gameResult = (board.sideToMove == Color::White) ? GameResult::BlackWinsCheckmate
                                                        : GameResult::WhiteWinsCheckmate;
    } else if (board.isStalemate()) {
        gameResult = GameResult::Stalemate;
    } else if (board.insufficientMaterial() || board.halfmoveClock >= 100) {
        gameResult = GameResult::Draw;
    }
}

std::string ChessGame::statusText() const {
    switch (gameResult) {
        case GameResult::WhiteWinsCheckmate: return "CHECKMATE!  WHITE WINS!";
        case GameResult::BlackWinsCheckmate: return "CHECKMATE!  BLACK WINS!";
        case GameResult::WhiteWinsTimeout:   return "TIME OUT!  WHITE WINS!";
        case GameResult::BlackWinsTimeout:   return "TIME OUT!  BLACK WINS!";
        case GameResult::Stalemate:          return "STALEMATE!  DRAW";
        case GameResult::Draw:               return "DRAW!";
        default: break;
    }
    std::string s = (board.sideToMove == Color::White) ? "White's Turn" : "Black's Turn";
    if (board.inCheck(board.sideToMove)) s += "  -  CHECK!";
    return s;
}
