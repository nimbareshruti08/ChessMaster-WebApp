// ============================================================================
// Chess Master - C++ Engine WebAssembly / JS Implementation
// ----------------------------------------------------------------------------
// DATA STRUCTURES PRESERVED FROM C++ DESKTOP CODEBASE:
//   1. 2D ARRAY    -> Board.squares[8][8]     (the 8x8 chessboard matrix)
//   2. LINKED LIST -> MoveLinkedList          (custom head/tail pointers, MoveNode)
//   3. STACK       -> MoveStack               (custom top pointer, StackNode, BoardState)
//
// ALGORITHMS PRESERVED FROM C++ DESKTOP CODEBASE:
//   * Full legal move generator with King safety validation
//   * Standard Algebraic Notation (SAN) disambiguation engine
//   * AI Minimax Search with Alpha-Beta Pruning (Depth 3) & Piece-Square Tables
//   * Handled Special Moves: Castling (O-O, O-O-O), En Passant, Pawn Promotion
//   * Draws: 50-move rule, Stalemate, Insufficient Material
// ============================================================================

export const PieceType = {
    None: 0,
    Pawn: 1,
    Knight: 2,
    Bishop: 3,
    Rook: 4,
    Queen: 5,
    King: 6
};

export const Color = {
    None: 0,
    White: 1,
    Black: 2
};

export function oppositeColor(c) {
    return c === Color.White ? Color.Black : (c === Color.Black ? Color.White : Color.None);
}

export class Piece {
    constructor(type = PieceType.None, color = Color.None) {
        this.type = type;
        this.color = color;
    }

    isEmpty() {
        return this.type === PieceType.None;
    }

    value() {
        switch (this.type) {
            case PieceType.Pawn:   return 100;
            case PieceType.Knight: return 320;
            case PieceType.Bishop: return 330;
            case PieceType.Rook:   return 500;
            case PieceType.Queen:  return 900;
            case PieceType.King:   return 20000;
            default:                return 0;
        }
    }

    letter() {
        let c = ' ';
        switch (this.type) {
            case PieceType.Pawn:   c = 'P'; break;
            case PieceType.Knight: c = 'N'; break;
            case PieceType.Bishop: c = 'B'; break;
            case PieceType.Rook:   c = 'R'; break;
            case PieceType.Queen:  c = 'Q'; break;
            case PieceType.King:   c = 'K'; break;
            default: return ' ';
        }
        if (this.color === Color.Black) c = c.toLowerCase();
        return c;
    }

    glyph() {
        const base = (this.color === Color.White) ? 0x2654 : 0x265A;
        switch (this.type) {
            case PieceType.King:   return String.fromCodePoint(base + 0);
            case PieceType.Queen:  return String.fromCodePoint(base + 1);
            case PieceType.Rook:   return String.fromCodePoint(base + 2);
            case PieceType.Bishop: return String.fromCodePoint(base + 3);
            case PieceType.Knight: return String.fromCodePoint(base + 4);
            case PieceType.Pawn:   return String.fromCodePoint(base + 5);
            default:                return ' ';
        }
    }

    clone() {
        return new Piece(this.type, this.color);
    }
}

export function pieceLetterSAN(t) {
    switch (t) {
        case PieceType.Knight: return "N";
        case PieceType.Bishop: return "B";
        case PieceType.Rook:   return "R";
        case PieceType.Queen:  return "Q";
        case PieceType.King:   return "K";
        default:                return "";
    }
}

export class Move {
    constructor(fromRow = -1, fromCol = -1, toRow = -1, toCol = -1) {
        this.fromRow = fromRow;
        this.fromCol = fromCol;
        this.toRow = toRow;
        this.toCol = toCol;
        this.moved = new Piece();
        this.captured = new Piece();
        this.promotion = PieceType.None;
        this.isCastleKingSide = false;
        this.isCastleQueenSide = false;
        this.isEnPassant = false;
        this.notation = "";
    }

    isValid() {
        return this.fromRow !== -1 && this.fromCol !== -1 && this.toRow !== -1 && this.toCol !== -1;
    }

    static squareName(row, col) {
        if (row < 0 || row > 7 || col < 0 || col > 7) return "??";
        return String.fromCharCode('a'.charCodeAt(0) + col) + (8 - row);
    }

    clone() {
        const m = new Move(this.fromRow, this.fromCol, this.toRow, this.toCol);
        m.moved = this.moved.clone();
        m.captured = this.captured.clone();
        m.promotion = this.promotion;
        m.isCastleKingSide = this.isCastleKingSide;
        m.isCastleQueenSide = this.isCastleQueenSide;
        m.isEnPassant = this.isEnPassant;
        m.notation = this.notation;
        return m;
    }
}

// ----------------------------------------------------------------------------
// LINKED LIST: Stores complete move history with dynamically linked MoveNodes.
// Exactly mirrors C++ `MoveLinkedList.h` and `MoveLinkedList.cpp`.
// ----------------------------------------------------------------------------
export class MoveNode {
    constructor(move) {
        this.move = move.clone();
        this.next = null;
    }
}

export class MoveLinkedList {
    constructor() {
        this.head = null;
        this.tail = null;
        this.count = 0;
    }

    addMove(m) {
        const node = new MoveNode(m);
        if (this.head === null) {
            this.head = node;
            this.tail = node;
        } else {
            this.tail.next = node;
            this.tail = node;
        }
        this.count++;
    }

    removeLastMove() {
        if (this.head === null) return false;

        if (this.head === this.tail) {
            this.head = null;
            this.tail = null;
            this.count = 0;
            return true;
        }

        let cur = this.head;
        while (cur.next !== this.tail) {
            cur = cur.next;
        }
        this.tail = cur;
        this.tail.next = null;
        this.count--;
        return true;
    }

    getLastMove() {
        if (this.tail === null) return null;
        return this.tail.move.clone();
    }

    clearHistory() {
        let cur = this.head;
        while (cur !== null) {
            let next = cur.next;
            cur.next = null;
            cur = next;
        }
        this.head = null;
        this.tail = null;
        this.count = 0;
    }

    traverseHistory() {
        const result = [];
        let cur = this.head;
        while (cur !== null) {
            result.push(cur.move.clone());
            cur = cur.next;
        }
        return result;
    }

    size() {
        return this.count;
    }

    empty() {
        return this.head === null;
    }
}

// ----------------------------------------------------------------------------
// STACK: Stores previous board states for UNDO.
// Exactly mirrors C++ `MoveStack.h` and `MoveStack.cpp`.
// ----------------------------------------------------------------------------
export class BoardState {
    constructor(board, whiteCaptured, blackCaptured, whiteTime, blackTime) {
        this.board = board.clone();
        this.whiteCaptured = whiteCaptured.map(p => p.clone());
        this.blackCaptured = blackCaptured.map(p => p.clone());
        this.whiteTime = whiteTime;
        this.blackTime = blackTime;
    }
}

export class StackNode {
    constructor(state) {
        this.state = state;
        this.next = null;
    }
}

export class MoveStack {
    constructor() {
        this.top = null;
        this.count = 0;
    }

    push(state) {
        const node = new StackNode(state);
        node.next = this.top;
        this.top = node;
        this.count++;
    }

    pop() {
        if (this.top === null) return null;
        const node = this.top;
        const state = node.state;
        this.top = node.next;
        this.count--;
        return state;
    }

    peek() {
        if (this.top === null) return null;
        return this.top.state;
    }

    isEmpty() {
        return this.top === null;
    }

    size() {
        return this.count;
    }

    clear() {
        while (this.top !== null) {
            let next = this.top.next;
            this.top.next = null;
            this.top = next;
        }
        this.count = 0;
    }
}

// ----------------------------------------------------------------------------
// BOARD: 2D Array `squares[8][8]` + Rule Engine
// Exactly mirrors C++ `Board.h` and `Board.cpp`.
// ----------------------------------------------------------------------------
const KNIGHT_DIRS = [[-2,-1],[-2,1],[-1,-2],[-1,2],[1,-2],[1,2],[2,-1],[2,1]];
const KING_DIRS   = [[-1,-1],[-1,0],[-1,1],[0,-1],[0,1],[1,-1],[1,0],[1,1]];
const ROOK_DIRS   = [[-1,0],[1,0],[0,-1],[0,1]];
const BISHOP_DIRS = [[-1,-1],[-1,1],[1,-1],[1,1]];

export class Board {
    constructor() {
        // 2D ARRAY: chessboard matrix squares[8][8]
        this.squares = Array.from({ length: 8 }, () => Array.from({ length: 8 }, () => new Piece()));
        this.sideToMove = Color.White;
        this.whiteCanCastleKing = true;
        this.whiteCanCastleQueen = true;
        this.blackCanCastleKing = true;
        this.blackCanCastleQueen = true;
        this.epRow = -1;
        this.epCol = -1;
        this.halfmoveClock = 0;
        this.fullmoveNumber = 1;
        this.reset();
    }

    reset() {
        for (let r = 0; r < 8; ++r) {
            for (let c = 0; c < 8; ++c) {
                this.squares[r][c] = new Piece();
            }
        }

        const backRank = [
            PieceType.Rook, PieceType.Knight, PieceType.Bishop,
            PieceType.Queen, PieceType.King, PieceType.Bishop,
            PieceType.Knight, PieceType.Rook
        ];

        for (let c = 0; c < 8; ++c) {
            this.squares[0][c] = new Piece(backRank[c], Color.Black);
            this.squares[1][c] = new Piece(PieceType.Pawn, Color.Black);
            this.squares[6][c] = new Piece(PieceType.Pawn, Color.White);
            this.squares[7][c] = new Piece(backRank[c], Color.White);
        }

        this.sideToMove = Color.White;
        this.whiteCanCastleKing = true;
        this.whiteCanCastleQueen = true;
        this.blackCanCastleKing = true;
        this.blackCanCastleQueen = true;
        this.epRow = -1;
        this.epCol = -1;
        this.halfmoveClock = 0;
        this.fullmoveNumber = 1;
    }

    static inside(r, c) {
        return r >= 0 && r < 8 && c >= 0 && c < 8;
    }

    at(r, c) {
        return this.squares[r][c];
    }

    clone() {
        const b = new Board();
        for (let r = 0; r < 8; ++r) {
            for (let c = 0; c < 8; ++c) {
                b.squares[r][c] = this.squares[r][c].clone();
            }
        }
        b.sideToMove = this.sideToMove;
        b.whiteCanCastleKing = this.whiteCanCastleKing;
        b.whiteCanCastleQueen = this.whiteCanCastleQueen;
        b.blackCanCastleKing = this.blackCanCastleKing;
        b.blackCanCastleQueen = this.blackCanCastleQueen;
        b.epRow = this.epRow;
        b.epCol = this.epCol;
        b.halfmoveClock = this.halfmoveClock;
        b.fullmoveNumber = this.fullmoveNumber;
        return b;
    }

    addSlidingMoves(r, c, side, dirs, out) {
        for (let d = 0; d < dirs.length; ++d) {
            let nr = r + dirs[d][0], nc = c + dirs[d][1];
            while (Board.inside(nr, nc)) {
                const target = this.squares[nr][nc];
                if (target.isEmpty()) {
                    const m = new Move(r, c, nr, nc);
                    m.moved = this.squares[r][c].clone();
                    out.push(m);
                } else {
                    if (target.color !== side) {
                        const m = new Move(r, c, nr, nc);
                        m.moved = this.squares[r][c].clone();
                        m.captured = target.clone();
                        out.push(m);
                    }
                    break;
                }
                nr += dirs[d][0];
                nc += dirs[d][1];
            }
        }
    }

    addStepMoves(r, c, side, dirs, out) {
        for (let d = 0; d < dirs.length; ++d) {
            let nr = r + dirs[d][0], nc = c + dirs[d][1];
            if (!Board.inside(nr, nc)) continue;
            const target = this.squares[nr][nc];
            if (!target.isEmpty() && target.color === side) continue;
            const m = new Move(r, c, nr, nc);
            m.moved = this.squares[r][c].clone();
            m.captured = target.clone();
            out.push(m);
        }
    }

    addPawnMoves(r, c, side, out) {
        const dir = (side === Color.White) ? -1 : 1;
        const startRow = (side === Color.White) ? 6 : 1;
        const promoRow = (side === Color.White) ? 0 : 7;

        const pushMaybePromo = (m) => {
            m.moved = this.squares[r][c].clone();
            if (m.toRow === promoRow) {
                const promos = [PieceType.Queen, PieceType.Rook, PieceType.Bishop, PieceType.Knight];
                for (let i = 0; i < 4; ++i) {
                    const pm = m.clone();
                    pm.promotion = promos[i];
                    out.push(pm);
                }
            } else {
                out.push(m);
            }
        };

        let nr = r + dir;
        if (Board.inside(nr, c) && this.squares[nr][c].isEmpty()) {
            pushMaybePromo(new Move(r, c, nr, c));
            let nr2 = r + 2 * dir;
            if (r === startRow && Board.inside(nr2, c) && this.squares[nr2][c].isEmpty()) {
                const m = new Move(r, c, nr2, c);
                m.moved = this.squares[r][c].clone();
                out.push(m);
            }
        }

        for (let dc = -1; dc <= 1; dc += 2) {
            let ncol = c + dc;
            if (!Board.inside(nr, ncol)) continue;
            const target = this.squares[nr][ncol];
            if (!target.isEmpty() && target.color !== side) {
                const m = new Move(r, c, nr, ncol);
                m.captured = target.clone();
                pushMaybePromo(m);
            } else if (target.isEmpty() && nr === this.epRow && ncol === this.epCol) {
                const m = new Move(r, c, nr, ncol);
                m.moved = this.squares[r][c].clone();
                m.isEnPassant = true;
                m.captured = new Piece(PieceType.Pawn, oppositeColor(side));
                out.push(m);
            }
        }
    }

    addCastlingMoves(side, out) {
        const row = (side === Color.White) ? 7 : 0;
        const canKing = (side === Color.White) ? this.whiteCanCastleKing : this.blackCanCastleKing;
        const canQueen = (side === Color.White) ? this.whiteCanCastleQueen : this.blackCanCastleQueen;

        const king = this.squares[row][4];
        if (king.type !== PieceType.King || king.color !== side) return;
        if (this.inCheck(side)) return;

        const enemy = oppositeColor(side);

        if (canKing && this.squares[row][5].isEmpty() && this.squares[row][6].isEmpty() &&
            this.squares[row][7].type === PieceType.Rook && this.squares[row][7].color === side &&
            !this.isSquareAttacked(row, 5, enemy) && !this.isSquareAttacked(row, 6, enemy)) {
            const m = new Move(row, 4, row, 6);
            m.moved = king.clone();
            m.isCastleKingSide = true;
            out.push(m);
        }

        if (canQueen && this.squares[row][3].isEmpty() && this.squares[row][2].isEmpty() &&
            this.squares[row][1].isEmpty() &&
            this.squares[row][0].type === PieceType.Rook && this.squares[row][0].color === side &&
            !this.isSquareAttacked(row, 3, enemy) && !this.isSquareAttacked(row, 2, enemy)) {
            const m = new Move(row, 4, row, 2);
            m.moved = king.clone();
            m.isCastleQueenSide = true;
            out.push(m);
        }
    }

    generatePseudoMoves(side, out) {
        for (let r = 0; r < 8; ++r) {
            for (let c = 0; c < 8; ++c) {
                const p = this.squares[r][c];
                if (p.isEmpty() || p.color !== side) continue;
                switch (p.type) {
                    case PieceType.Pawn:   this.addPawnMoves(r, c, side, out); break;
                    case PieceType.Knight: this.addStepMoves(r, c, side, KNIGHT_DIRS, out); break;
                    case PieceType.King:   this.addStepMoves(r, c, side, KING_DIRS, out); break;
                    case PieceType.Bishop: this.addSlidingMoves(r, c, side, BISHOP_DIRS, out); break;
                    case PieceType.Rook:   this.addSlidingMoves(r, c, side, ROOK_DIRS, out); break;
                    case PieceType.Queen:
                        this.addSlidingMoves(r, c, side, ROOK_DIRS, out);
                        this.addSlidingMoves(r, c, side, BISHOP_DIRS, out);
                        break;
                    default: break;
                }
            }
        }
        this.addCastlingMoves(side, out);
    }

    generateLegalMovesFor(side, out) {
        const pseudo = [];
        this.generatePseudoMoves(side, pseudo);

        for (let i = 0; i < pseudo.length; ++i) {
            const copy = this.clone();
            copy.applyMove(pseudo[i]);
            if (!copy.inCheck(side)) out.push(pseudo[i]);
        }
    }

    generateLegalMoves(out) {
        this.generateLegalMovesFor(this.sideToMove, out);
    }

    isSquareAttacked(r, c, by) {
        const dir = (by === Color.White) ? 1 : -1;
        for (let dc = -1; dc <= 1; dc += 2) {
            let pr = r + dir, pc = c + dc;
            if (Board.inside(pr, pc)) {
                const p = this.squares[pr][pc];
                if (p.type === PieceType.Pawn && p.color === by) return true;
            }
        }

        for (let d = 0; d < 8; ++d) {
            let nr = r + KNIGHT_DIRS[d][0], nc = c + KNIGHT_DIRS[d][1];
            if (!Board.inside(nr, nc)) continue;
            const p = this.squares[nr][nc];
            if (p.type === PieceType.Knight && p.color === by) return true;
        }

        for (let d = 0; d < 8; ++d) {
            let nr = r + KING_DIRS[d][0], nc = c + KING_DIRS[d][1];
            if (!Board.inside(nr, nc)) continue;
            const p = this.squares[nr][nc];
            if (p.type === PieceType.King && p.color === by) return true;
        }

        for (let d = 0; d < 4; ++d) {
            let nr = r + ROOK_DIRS[d][0], nc = c + ROOK_DIRS[d][1];
            while (Board.inside(nr, nc)) {
                const p = this.squares[nr][nc];
                if (!p.isEmpty()) {
                    if (p.color === by && (p.type === PieceType.Rook || p.type === PieceType.Queen))
                        return true;
                    break;
                }
                nr += ROOK_DIRS[d][0];
                nc += ROOK_DIRS[d][1];
            }
        }

        for (let d = 0; d < 4; ++d) {
            let nr = r + BISHOP_DIRS[d][0], nc = c + BISHOP_DIRS[d][1];
            while (Board.inside(nr, nc)) {
                const p = this.squares[nr][nc];
                if (!p.isEmpty()) {
                    if (p.color === by && (p.type === PieceType.Bishop || p.type === PieceType.Queen))
                        return true;
                    break;
                }
                nr += BISHOP_DIRS[d][0];
                nc += BISHOP_DIRS[d][1];
            }
        }
        return false;
    }

    findKing(c) {
        for (let r = 0; r < 8; ++r) {
            for (let cc = 0; cc < 8; ++cc) {
                if (this.squares[r][cc].type === PieceType.King && this.squares[r][cc].color === c) {
                    return { row: r, col: cc };
                }
            }
        }
        return null;
    }

    inCheck(c) {
        const k = this.findKing(c);
        if (!k) return false;
        return this.isSquareAttacked(k.row, k.col, oppositeColor(c));
    }

    isCheckmate() {
        if (!this.inCheck(this.sideToMove)) return false;
        const moves = [];
        this.generateLegalMoves(moves);
        return moves.length === 0;
    }

    isStalemate() {
        if (this.inCheck(this.sideToMove)) return false;
        const moves = [];
        this.generateLegalMoves(moves);
        return moves.length === 0;
    }

    insufficientMaterial() {
        let minor = 0;
        for (let r = 0; r < 8; ++r) {
            for (let c = 0; c < 8; ++c) {
                const p = this.squares[r][c];
                if (p.isEmpty() || p.type === PieceType.King) continue;
                if (p.type === PieceType.Bishop || p.type === PieceType.Knight) { ++minor; continue; }
                return false;
            }
        }
        return minor <= 1;
    }

    applyMove(m) {
        const moving = this.squares[m.fromRow][m.fromCol].clone();
        const capture = !this.squares[m.toRow][m.toCol].isEmpty() || m.isEnPassant;

        if (m.isEnPassant) {
            const capturedRow = (moving.color === Color.White) ? m.toRow + 1 : m.toRow - 1;
            this.squares[capturedRow][m.toCol] = new Piece();
        }

        this.squares[m.toRow][m.toCol] = moving;
        this.squares[m.fromRow][m.fromCol] = new Piece();

        if (m.promotion !== PieceType.None) {
            this.squares[m.toRow][m.toCol] = new Piece(m.promotion, moving.color);
        }

        if (m.isCastleKingSide) {
            this.squares[m.toRow][5] = this.squares[m.toRow][7].clone();
            this.squares[m.toRow][7] = new Piece();
        } else if (m.isCastleQueenSide) {
            this.squares[m.toRow][3] = this.squares[m.toRow][0].clone();
            this.squares[m.toRow][0] = new Piece();
        }

        if (moving.type === PieceType.King) {
            if (moving.color === Color.White) this.whiteCanCastleKing = this.whiteCanCastleQueen = false;
            else                              this.blackCanCastleKing = this.blackCanCastleQueen = false;
        }
        if (m.fromRow === 7 && m.fromCol === 0) this.whiteCanCastleQueen = false;
        if (m.fromRow === 7 && m.fromCol === 7) this.whiteCanCastleKing  = false;
        if (m.fromRow === 0 && m.fromCol === 0) this.blackCanCastleQueen = false;
        if (m.fromRow === 0 && m.fromCol === 7) this.blackCanCastleKing  = false;
        if (m.toRow === 7 && m.toCol === 0) this.whiteCanCastleQueen = false;
        if (m.toRow === 7 && m.toCol === 7) this.whiteCanCastleKing  = false;
        if (m.toRow === 0 && m.toCol === 0) this.blackCanCastleQueen = false;
        if (m.toRow === 0 && m.toCol === 7) this.blackCanCastleKing  = false;

        this.epRow = -1;
        this.epCol = -1;
        if (moving.type === PieceType.Pawn && Math.abs(m.toRow - m.fromRow) === 2) {
            this.epRow = Math.floor((m.fromRow + m.toRow) / 2);
            this.epCol = m.fromCol;
        }

        if (moving.type === PieceType.Pawn || capture) this.halfmoveClock = 0;
        else ++this.halfmoveClock;

        if (this.sideToMove === Color.Black) ++this.fullmoveNumber;
        this.sideToMove = oppositeColor(this.sideToMove);
    }

    moveToSAN(m) {
        if (m.isCastleKingSide || m.isCastleQueenSide) {
            let s = m.isCastleKingSide ? "O-O" : "O-O-O";
            const copy = this.clone();
            copy.applyMove(m);
            if (copy.isCheckmate()) s += "#";
            else if (copy.inCheck(copy.sideToMove)) s += "+";
            return s;
        }

        const moving = this.squares[m.fromRow][m.fromCol];
        const capture = !this.squares[m.toRow][m.toCol].isEmpty() || m.isEnPassant;
        let s = "";

        if (moving.type === PieceType.Pawn) {
            if (capture) {
                s += String.fromCharCode('a'.charCodeAt(0) + m.fromCol);
                s += "x";
            }
        } else {
            s += pieceLetterSAN(moving.type);

            const legal = [];
            this.generateLegalMovesFor(moving.color, legal);
            let needFile = false, needRank = false, ambiguous = false;

            for (let i = 0; i < legal.length; ++i) {
                const o = legal[i];
                if (o.toRow !== m.toRow || o.toCol !== m.toCol) continue;
                if (o.fromRow === m.fromRow && o.fromCol === m.fromCol) continue;
                if (this.squares[o.fromRow][o.fromCol].type !== moving.type) continue;
                ambiguous = true;
                if (o.fromCol === m.fromCol) needRank = true;
                else needFile = true;
            }

            if (ambiguous) {
                if (needFile || !needRank) s += String.fromCharCode('a'.charCodeAt(0) + m.fromCol);
                if (needRank)              s += String.fromCharCode('8'.charCodeAt(0) - m.fromRow);
            }
            if (capture) s += "x";
        }

        s += Move.squareName(m.toRow, m.toCol);
        if (m.promotion !== PieceType.None) s += "=" + pieceLetterSAN(m.promotion);

        const copy = this.clone();
        copy.applyMove(m);
        if (copy.isCheckmate()) s += "#";
        else if (copy.inCheck(copy.sideToMove)) s += "+";
        return s;
    }
}

// ----------------------------------------------------------------------------
// ChessAI: Computer Opponent (Easy / Medium / Hard)
// Exactly mirrors C++ `ChessAI.h` and `ChessAI.cpp`.
// ----------------------------------------------------------------------------
export const Difficulty = { Easy: 0, Medium: 1, Hard: 2 };

const PAWN_PST = [
    [ 0,  0,  0,  0,  0,  0,  0,  0],
    [50, 50, 50, 50, 50, 50, 50, 50],
    [10, 10, 20, 30, 30, 20, 10, 10],
    [ 5,  5, 10, 25, 25, 10,  5,  5],
    [ 0,  0,  0, 20, 20,  0,  0,  0],
    [ 5, -5,-10,  0,  0,-10, -5,  5],
    [ 5, 10, 10,-20,-20, 10, 10,  5],
    [ 0,  0,  0,  0,  0,  0,  0,  0]
];
const KNIGHT_PST = [
    [-50,-40,-30,-30,-30,-30,-40,-50],
    [-40,-20,  0,  0,  0,  0,-20,-40],
    [-30,  0, 10, 15, 15, 10,  0,-30],
    [-30,  5, 15, 20, 20, 15,  5,-30],
    [-30,  0, 15, 20, 20, 15,  0,-30],
    [-30,  5, 10, 15, 15, 10,  5,-30],
    [-40,-20,  0,  5,  5,  0,-20,-40],
    [-50,-40,-30,-30,-30,-30,-40,-50]
];
const BISHOP_PST = [
    [-20,-10,-10,-10,-10,-10,-10,-20],
    [-10,  0,  0,  0,  0,  0,  0,-10],
    [-10,  0,  5, 10, 10,  5,  0,-10],
    [-10,  5,  5, 10, 10,  5,  5,-10],
    [-10,  0, 10, 10, 10, 10,  0,-10],
    [-10, 10, 10, 10, 10, 10, 10,-10],
    [-10,  5,  0,  0,  0,  0,  5,-10],
    [-20,-10,-10,-10,-10,-10,-10,-20]
];
const ROOK_PST = [
    [  0,  0,  0,  0,  0,  0,  0,  0],
    [  5, 10, 10, 10, 10, 10, 10,  5],
    [ -5,  0,  0,  0,  0,  0,  0, -5],
    [ -5,  0,  0,  0,  0,  0,  0, -5],
    [ -5,  0,  0,  0,  0,  0,  0, -5],
    [ -5,  0,  0,  0,  0,  0,  0, -5],
    [ -5,  0,  0,  0,  0,  0,  0, -5],
    [  0,  0,  0,  5,  5,  0,  0,  0]
];
const QUEEN_PST = [
    [-20,-10,-10, -5, -5,-10,-10,-20],
    [-10,  0,  0,  0,  0,  0,  0,-10],
    [-10,  0,  5,  5,  5,  5,  0,-10],
    [ -5,  0,  5,  5,  5,  5,  0, -5],
    [  0,  0,  5,  5,  5,  5,  0, -5],
    [-10,  5,  5,  5,  5,  5,  0,-10],
    [-10,  0,  5,  0,  0,  0,  0,-10],
    [-20,-10,-10, -5, -5,-10,-10,-20]
];
const KING_PST = [
    [-30,-40,-40,-50,-50,-40,-40,-30],
    [-30,-40,-40,-50,-50,-40,-40,-30],
    [-30,-40,-40,-50,-50,-40,-40,-30],
    [-30,-40,-40,-50,-50,-40,-40,-30],
    [-20,-30,-30,-40,-40,-30,-30,-20],
    [-10,-20,-20,-20,-20,-20,-20,-10],
    [ 20, 20,  0,  0,  0, 20, 20, 20],
    [ 20, 30, 10,  0,  0, 10, 30, 20]
];

function pstValue(t, color, r, c) {
    const row = (color === Color.White) ? r : 7 - r;
    switch (t) {
        case PieceType.Pawn:   return PAWN_PST[row][c];
        case PieceType.Knight: return KNIGHT_PST[row][c];
        case PieceType.Bishop: return BISHOP_PST[row][c];
        case PieceType.Rook:   return ROOK_PST[row][c];
        case PieceType.Queen:  return QUEEN_PST[row][c];
        case PieceType.King:   return KING_PST[row][c];
        default:                return 0;
    }
}

export class ChessAI {
    constructor(d = Difficulty.Medium) {
        this.difficulty = d;
    }

    setDifficulty(d) {
        this.difficulty = d;
    }

    getDifficulty() {
        return this.difficulty;
    }

    static evaluate(board) {
        let score = 0;
        for (let r = 0; r < 8; ++r) {
            for (let c = 0; c < 8; ++c) {
                const p = board.at(r, c);
                if (p.isEmpty()) continue;
                let v = p.value() + pstValue(p.type, p.color, r, c);
                score += (p.color === Color.White) ? v : -v;
            }
        }
        return score;
    }

    chooseMove(board) {
        switch (this.difficulty) {
            case Difficulty.Easy:   return this.chooseRandomMove(board);
            case Difficulty.Medium: return this.chooseGreedyMove(board);
            default:                 return this.chooseSearchMove(board);
        }
    }

    chooseRandomMove(board) {
        const moves = [];
        board.generateLegalMoves(moves);
        if (moves.length === 0) return new Move();

        const captures = moves.filter(m => !m.captured.isEmpty());
        if (captures.length > 0 && Math.random() < 0.4) {
            return captures[Math.floor(Math.random() * captures.length)];
        }
        return moves[Math.floor(Math.random() * moves.length)];
    }

    chooseGreedyMove(board) {
        const moves = [];
        board.generateLegalMoves(moves);
        if (moves.length === 0) return new Move();

        const maximizing = (board.sideToMove === Color.White);
        let bestScore = maximizing ? -1000000 : 1000000;
        let best = [];

        for (let i = 0; i < moves.length; ++i) {
            const copy = board.clone();
            copy.applyMove(moves[i]);

            let score = ChessAI.evaluate(copy);
            if (copy.isCheckmate()) score += maximizing ? 100000 : -100000;

            if ((maximizing && score > bestScore) || (!maximizing && score < bestScore)) {
                bestScore = score;
                best = [moves[i]];
            } else if (score === bestScore) {
                best.push(moves[i]);
            }
        }
        return best[Math.floor(Math.random() * best.length)];
    }

    static minimax(board, depth, alpha, beta, maximizing) {
        const moves = [];
        board.generateLegalMoves(moves);

        if (moves.length === 0) {
            if (board.inCheck(board.sideToMove))
                return maximizing ? -900000 - depth : 900000 + depth;
            return 0;
        }
        if (depth === 0) return ChessAI.evaluate(board);

        moves.sort((a, b) => b.captured.value() - a.captured.value());

        if (maximizing) {
            let best = -1000000;
            for (let i = 0; i < moves.length; ++i) {
                const copy = board.clone();
                copy.applyMove(moves[i]);
                best = Math.max(best, ChessAI.minimax(copy, depth - 1, alpha, beta, false));
                alpha = Math.max(alpha, best);
                if (beta <= alpha) break;
            }
            return best;
        }

        let best = 1000000;
        for (let i = 0; i < moves.length; ++i) {
            const copy = board.clone();
            copy.applyMove(moves[i]);
            best = Math.min(best, ChessAI.minimax(copy, depth - 1, alpha, beta, true));
            beta = Math.min(beta, best);
            if (beta <= alpha) break;
        }
        return best;
    }

    chooseSearchMove(board) {
        const moves = [];
        board.generateLegalMoves(moves);
        if (moves.length === 0) return new Move();

        const depth = 3;
        const maximizing = (board.sideToMove === Color.White);
        let bestScore = maximizing ? -1000000 : 1000000;
        let bestMove = moves[0];

        moves.sort((a, b) => b.captured.value() - a.captured.value());

        for (let i = 0; i < moves.length; ++i) {
            const copy = board.clone();
            copy.applyMove(moves[i]);
            let score = ChessAI.minimax(copy, depth - 1, -1000000, 1000000, !maximizing);
            if ((maximizing && score > bestScore) || (!maximizing && score < bestScore)) {
                bestScore = score;
                bestMove = moves[i];
            }
        }
        return bestMove;
    }
}

// ----------------------------------------------------------------------------
// TIMER: Simple chess clock for both players
// Exactly mirrors C++ `Timer.h` and `Timer.cpp`.
// ----------------------------------------------------------------------------
export class Timer {
    constructor(minutes = 10) {
        this.baseMinutes = minutes;
        this.whiteSeconds = minutes * 60;
        this.blackSeconds = minutes * 60;
        this.reset();
    }

    setMinutes(minutes) {
        this.baseMinutes = minutes;
        this.reset();
    }

    reset() {
        this.whiteSeconds = this.baseMinutes * 60;
        this.blackSeconds = this.baseMinutes * 60;
    }

    tick(activeSide, deltaSeconds) {
        if (activeSide === Color.White) {
            this.whiteSeconds -= deltaSeconds;
            if (this.whiteSeconds < 0) this.whiteSeconds = 0;
        } else if (activeSide === Color.Black) {
            this.blackSeconds -= deltaSeconds;
            if (this.blackSeconds < 0) this.blackSeconds = 0;
        }
    }

    whiteTime() { return this.whiteSeconds; }
    blackTime() { return this.blackSeconds; }

    setTimes(w, b) {
        this.whiteSeconds = w;
        this.blackSeconds = b;
    }

    whiteFlagged() { return this.whiteSeconds <= 0; }
    blackFlagged() { return this.blackSeconds <= 0; }

    minutes() { return this.baseMinutes; }

    static format(seconds) {
        if (seconds < 0) seconds = 0;
        const total = Math.ceil(seconds);
        const m = Math.floor(total / 60);
        const s = total % 60;
        return `${m.toString().padStart(2, '0')}:${s.toString().padStart(2, '0')}`;
    }
}

// ----------------------------------------------------------------------------
// CHESSGAME: Game controller coordinating rules, history, undo, clock & AI
// Exactly mirrors C++ `ChessGame.h` and `ChessGame.cpp`.
// ----------------------------------------------------------------------------
export const GameMode = { Multiplayer: 0, VsComputer: 1 };

export const GameResult = {
    Ongoing: 0,
    WhiteWinsCheckmate: 1,
    BlackWinsCheckmate: 2,
    WhiteWinsTimeout: 3,
    BlackWinsTimeout: 4,
    Stalemate: 5,
    Draw: 6
};

export class ChessGame {
    constructor() {
        this.board = new Board();
        this.moveHistory = new MoveLinkedList(); // LINKED LIST
        this.undoStack = new MoveStack();         // STACK
        this.timer = new Timer(10);
        this.ai = new ChessAI(Difficulty.Medium);
        this.mode = GameMode.Multiplayer;
        this.gameResult = GameResult.Ongoing;
        this.capturedByWhite = [];
        this.capturedByBlack = [];
        this.newGame();
    }

    newGame() {
        this.board.reset();
        this.moveHistory.clearHistory(); // LINKED LIST cleared
        this.undoStack.clear();          // STACK cleared
        this.capturedByWhite = [];
        this.capturedByBlack = [];
        this.timer.reset();
        this.gameResult = GameResult.Ongoing;
    }

    setMode(m) { this.mode = m; }
    getMode() { return this.mode; }

    setDifficulty(d) { this.ai.setDifficulty(d); }
    getDifficulty() { return this.ai.getDifficulty(); }

    setMinutes(m) { this.timer.setMinutes(m); }

    snapshot() {
        return new BoardState(
            this.board,
            this.capturedByWhite,
            this.capturedByBlack,
            this.timer.whiteTime(),
            this.timer.blackTime()
        );
    }

    restore(s) {
        this.board = s.board.clone();
        this.capturedByWhite = s.whiteCaptured.map(p => p.clone());
        this.capturedByBlack = s.blackCaptured.map(p => p.clone());
        this.timer.setTimes(s.whiteTime, s.blackTime);
    }

    legalMovesFrom(row, col) {
        const all = [], result = [];
        this.board.generateLegalMoves(all);
        for (let i = 0; i < all.length; ++i) {
            if (all[i].fromRow === row && all[i].fromCol === col) {
                result.push(all[i]);
            }
        }
        return result;
    }

    makeMove(fromRow, fromCol, toRow, toCol, promotion = PieceType.Queen) {
        if (this.isOver()) return false;

        const legal = [];
        this.board.generateLegalMoves(legal);

        let chosen = null;
        for (let i = 0; i < legal.length; ++i) {
            const m = legal[i];
            if (m.fromRow === fromRow && m.fromCol === fromCol &&
                m.toRow === toRow && m.toCol === toCol) {
                if (m.promotion === PieceType.None || m.promotion === promotion) {
                    chosen = m;
                    break;
                }
            }
        }

        if (!chosen) return false;

        // STACK: Push state snapshot BEFORE move execution
        this.undoStack.push(this.snapshot());

        chosen.notation = this.board.moveToSAN(chosen);
        chosen.moved = this.board.at(fromRow, fromCol).clone();

        if (!chosen.captured.isEmpty()) {
            if (chosen.moved.color === Color.White) this.capturedByWhite.push(chosen.captured.clone());
            else                                    this.capturedByBlack.push(chosen.captured.clone());
        }

        this.board.applyMove(chosen);

        // LINKED LIST: Append move to history list
        this.moveHistory.addMove(chosen);

        this.checkGameOver();
        return true;
    }

    computerShouldMove() {
        return this.mode === GameMode.VsComputer && !this.isOver() && this.board.sideToMove !== Color.White;
    }

    playComputerMove() {
        if (!this.computerShouldMove()) return null;
        const m = this.ai.chooseMove(this.board);
        if (!m.isValid()) {
            this.checkGameOver();
            return null;
        }
        const promo = (m.promotion === PieceType.None) ? PieceType.Queen : m.promotion;
        this.makeMove(m.fromRow, m.fromCol, m.toRow, m.toCol, promo);
        return m;
    }

    update(deltaSeconds) {
        if (this.isOver()) return;
        if (this.moveHistory.size() === 0) return;

        this.timer.tick(this.board.sideToMove, deltaSeconds);

        if (this.timer.whiteFlagged())      this.gameResult = GameResult.BlackWinsTimeout;
        else if (this.timer.blackFlagged()) this.gameResult = GameResult.WhiteWinsTimeout;
    }

    undo() {
        const state = this.undoStack.pop(); // STACK POP
        if (!state) return false;

        this.restore(state);
        this.moveHistory.removeLastMove(); // LINKED LIST REMOVE TAIL
        this.gameResult = GameResult.Ongoing;

        if (this.mode === GameMode.VsComputer && this.board.sideToMove !== Color.White) {
            const state2 = this.undoStack.pop();
            if (state2) {
                this.restore(state2);
                this.moveHistory.removeLastMove();
            }
        }
        return true;
    }

    checkGameOver() {
        if (this.board.isCheckmate()) {
            this.gameResult = (this.board.sideToMove === Color.White) ? GameResult.BlackWinsCheckmate
                                                                    : GameResult.WhiteWinsCheckmate;
        } else if (this.board.isStalemate()) {
            this.gameResult = GameResult.Stalemate;
        } else if (this.board.insufficientMaterial() || this.board.halfmoveClock >= 100) {
            this.gameResult = GameResult.Draw;
        }
    }

    getBoard() { return this.board; }
    turn() { return this.board.sideToMove; }
    getTimer() { return this.timer; }
    result() { return this.gameResult; }
    isOver() { return this.gameResult !== GameResult.Ongoing; }
    inCheckNow() { return this.board.inCheck(this.board.sideToMove); }

    whiteCaptured() { return this.capturedByWhite; }
    blackCaptured() { return this.capturedByBlack; }

    history() { return this.moveHistory.traverseHistory(); }
    lastMove() { return this.moveHistory.getLastMove(); }

    humanColor() { return Color.White; }

    statusText() {
        switch (this.gameResult) {
            case GameResult.WhiteWinsCheckmate: return "CHECKMATE! WHITE WINS!";
            case GameResult.BlackWinsCheckmate: return "CHECKMATE! BLACK WINS!";
            case GameResult.WhiteWinsTimeout:   return "TIME OUT! WHITE WINS!";
            case GameResult.BlackWinsTimeout:   return "TIME OUT! BLACK WINS!";
            case GameResult.Stalemate:          return "STALEMATE! DRAW";
            case GameResult.Draw:               return "DRAW!";
            default: break;
        }
        let s = (this.board.sideToMove === Color.White) ? "White's Turn" : "Black's Turn";
        if (this.board.inCheck(this.board.sideToMove)) s += "  -  CHECK!";
        return s;
    }
}
