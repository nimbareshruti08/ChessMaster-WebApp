// ============================================================================
// Chess Master - Application Controller (Web UI + C++ Engine Integration)
// ============================================================================

import {
  ChessGame, GameMode, GameResult, Difficulty, Color, PieceType, Move
} from './cpp_engine.js';

// --- Web Audio Sound System ---
class SoundFX {
  constructor() {
    this.enabled = true;
    this.ctx = null;
  }

  init() {
    if (!this.ctx) {
      const AudioCtx = window.AudioContext || window.webkitAudioContext;
      if (AudioCtx) this.ctx = new AudioCtx();
    }
  }

  playMove() {
    if (!this.enabled) return;
    this.init();
    if (!this.ctx) return;
    const osc = this.ctx.createOscillator();
    const gain = this.ctx.createGain();
    osc.type = 'sine';
    osc.frequency.setValueAtTime(440, this.ctx.currentTime);
    osc.frequency.exponentialRampToValueAtTime(220, this.ctx.currentTime + 0.08);
    gain.gain.setValueAtTime(0.15, this.ctx.currentTime);
    gain.gain.exponentialRampToValueAtTime(0.01, this.ctx.currentTime + 0.08);
    osc.connect(gain);
    gain.connect(this.ctx.destination);
    osc.start();
    osc.stop(this.ctx.currentTime + 0.08);
  }

  playCapture() {
    if (!this.enabled) return;
    this.init();
    if (!this.ctx) return;
    const osc = this.ctx.createOscillator();
    const gain = this.ctx.createGain();
    osc.type = 'triangle';
    osc.frequency.setValueAtTime(180, this.ctx.currentTime);
    osc.frequency.exponentialRampToValueAtTime(60, this.ctx.currentTime + 0.12);
    gain.gain.setValueAtTime(0.3, this.ctx.currentTime);
    gain.gain.exponentialRampToValueAtTime(0.01, this.ctx.currentTime + 0.12);
    osc.connect(gain);
    gain.connect(this.ctx.destination);
    osc.start();
    osc.stop(this.ctx.currentTime + 0.12);
  }

  playCheck() {
    if (!this.enabled) return;
    this.init();
    if (!this.ctx) return;
    const osc = this.ctx.createOscillator();
    const gain = this.ctx.createGain();
    osc.type = 'sawtooth';
    osc.frequency.setValueAtTime(587.33, this.ctx.currentTime); // D5
    osc.frequency.setValueAtTime(880, this.ctx.currentTime + 0.08); // A5
    gain.gain.setValueAtTime(0.2, this.ctx.currentTime);
    gain.gain.exponentialRampToValueAtTime(0.01, this.ctx.currentTime + 0.2);
    osc.connect(gain);
    gain.connect(this.ctx.destination);
    osc.start();
    osc.stop(this.ctx.currentTime + 0.2);
  }

  playGameOver() {
    if (!this.enabled) return;
    this.init();
    if (!this.ctx) return;
    const now = this.ctx.currentTime;
    [523.25, 659.25, 783.99, 1046.50].forEach((freq, i) => {
      const osc = this.ctx.createOscillator();
      const gain = this.ctx.createGain();
      osc.type = 'sine';
      osc.frequency.setValueAtTime(freq, now + i * 0.1);
      gain.gain.setValueAtTime(0.2, now + i * 0.1);
      gain.gain.exponentialRampToValueAtTime(0.01, now + i * 0.1 + 0.3);
      osc.connect(gain);
      gain.connect(this.ctx.destination);
      osc.start(now + i * 0.1);
      osc.stop(now + i * 0.1 + 0.3);
    });
  }
}

// --- Main Web App State ---
class ChessApp {
  constructor() {
    this.game = new ChessGame();
    this.sound = new SoundFX();

    this.selectedRow = -1;
    this.selectedCol = -1;
    this.selectedMoves = [];
    this.pendingPromoMove = null;

    this.lastTime = performance.now();
    this.aiThinking = false;
    this.confirmAction = null;

    this.initUI();
    this.initEventListeners();
    this.startClockLoop();
  }

  initUI() {
    this.chessboardEl = document.getElementById('chessboard');
    this.buildBoardGrid();
    this.updateBoardUI();
  }

  buildBoardGrid() {
    this.chessboardEl.innerHTML = '';
    for (let r = 0; r < 8; ++r) {
      for (let c = 0; c < 8; ++c) {
        const sq = document.createElement('div');
        sq.className = `sq ${(r + c) % 2 === 0 ? 'light' : 'dark'}`;
        sq.dataset.row = r;
        sq.dataset.col = c;

        // Rank / File coordinates overlay
        if (c === 0) {
          const rankLabel = document.createElement('span');
          rankLabel.className = 'coord rank';
          rankLabel.textContent = (8 - r).toString();
          sq.appendChild(rankLabel);
        }
        if (r === 7) {
          const fileLabel = document.createElement('span');
          fileLabel.className = 'coord file';
          fileLabel.textContent = String.fromCharCode('a'.charCodeAt(0) + c);
          sq.appendChild(fileLabel);
        }

        sq.addEventListener('click', () => this.handleSquareClick(r, c));
        this.chessboardEl.appendChild(sq);
      }
    }
  }

  // --- Input & Move Processing ---
  handleSquareClick(row, col) {
    if (this.game.isOver() || this.aiThinking) return;

    // If already selected, check if clicked target square is a legal move
    if (this.selectedRow !== -1 && this.selectedCol !== -1) {
      const move = this.selectedMoves.find(m => m.toRow === row && m.toCol === col);
      if (move) {
        this.executeHumanMove(move);
        return;
      }
    }

    // Select piece if it belongs to current player
    const piece = this.game.getBoard().at(row, col);
    if (!piece.isEmpty() && piece.color === this.game.turn()) {
      // In vs Computer mode, human can only move White
      if (this.game.getMode() === GameMode.VsComputer && piece.color !== this.game.humanColor()) {
        return;
      }
      this.selectedRow = row;
      this.selectedCol = col;
      this.selectedMoves = this.game.legalMovesFrom(row, col);
    } else {
      this.resetSelection();
    }

    this.updateBoardUI();
  }

  resetSelection() {
    this.selectedRow = -1;
    this.selectedCol = -1;
    this.selectedMoves = [];
  }

  executeHumanMove(move) {
    const isPromo = (move.moved.type === PieceType.Pawn && (move.toRow === 0 || move.toRow === 7));

    if (isPromo) {
      this.pendingPromoMove = move;
      document.getElementById('promo-modal').classList.add('active');
      return;
    }

    this.commitMove(move.fromRow, move.fromCol, move.toRow, move.toCol, PieceType.Queen);
  }

  commitMove(fromR, fromC, toR, toC, promoType) {
    const board = this.game.getBoard();
    const targetPiece = board.at(toR, toC);
    const isCapture = !targetPiece.isEmpty() || (board.at(fromR, fromC).type === PieceType.Pawn && toC !== fromC && targetPiece.isEmpty());

    const success = this.game.makeMove(fromR, fromC, toR, toC, promoType);
    if (success) {
      this.resetSelection();

      if (isCapture) this.sound.playCapture();
      else this.sound.playMove();

      if (this.game.inCheckNow()) this.sound.playCheck();

      this.updateBoardUI();
      this.updateGameUI();

      if (this.game.isOver()) {
        this.handleGameOver();
      } else if (this.game.computerShouldMove()) {
        this.scheduleAIMove();
      }
    }
  }

  scheduleAIMove() {
    this.aiThinking = true;
    this.updateStatusText("AI is thinking...");

    // Slight delay to allow UI render & smooth pacing
    setTimeout(() => {
      const boardBefore = this.game.getBoard().clone();
      const compMove = this.game.playComputerMove();
      this.aiThinking = false;

      if (compMove) {
        const isCap = !boardBefore.at(compMove.toRow, compMove.toCol).isEmpty() || compMove.isEnPassant;
        if (isCap) this.sound.playCapture();
        else this.sound.playMove();

        if (this.game.inCheckNow()) this.sound.playCheck();
      }

      this.updateBoardUI();
      this.updateGameUI();

      if (this.game.isOver()) {
        this.handleGameOver();
      }
    }, 250);
  }

  // --- Clock & Game Loop ---
  startClockLoop() {
    const loop = (now) => {
      const delta = (now - this.lastTime) / 1000;
      this.lastTime = now;

      if (!this.game.isOver()) {
        const resultBefore = this.game.result();
        this.game.update(delta);

        if (this.game.isOver() && resultBefore === GameResult.Ongoing) {
          this.handleGameOver();
        } else {
          this.updateClocksUI();
        }
      }

      requestAnimationFrame(loop);
    };
    requestAnimationFrame(loop);
  }

  // --- UI Update Renderers ---
  updateBoardUI() {
    const board = this.game.getBoard();
    const lastMove = this.game.lastMove();
    const kingPos = board.inCheck(board.sideToMove) ? board.findKing(board.sideToMove) : null;

    const squares = this.chessboardEl.querySelectorAll('.sq');
    squares.forEach(sq => {
      const r = parseInt(sq.dataset.row, 10);
      const c = parseInt(sq.dataset.col, 10);
      const piece = board.at(r, c);

      // Reset classes
      sq.classList.remove('selected', 'lastmove', 'in-check');

      // Clear previous indicators/pieces except coords
      const coords = sq.querySelectorAll('.coord');
      sq.innerHTML = '';
      coords.forEach(cc => sq.appendChild(cc));

      // Selected highlight
      if (r === this.selectedRow && c === this.selectedCol) {
        sq.classList.add('selected');
      }

      // Last move highlight
      if (lastMove && ((r === lastMove.fromRow && c === lastMove.fromCol) || (r === lastMove.toRow && c === lastMove.toCol))) {
        sq.classList.add('lastmove');
      }

      // King in check highlight
      if (kingPos && r === kingPos.row && c === kingPos.col) {
        sq.classList.add('in-check');
      }

      // Render Piece
      if (!piece.isEmpty()) {
        const pieceEl = document.createElement('span');
        pieceEl.className = `piece ${piece.color === Color.White ? 'white' : 'black'}`;
        pieceEl.textContent = piece.glyph();
        sq.appendChild(pieceEl);
      }

      // Legal move indicators
      if (this.selectedRow !== -1) {
        const move = this.selectedMoves.find(m => m.toRow === r && m.toCol === c);
        if (move) {
          if (piece.isEmpty() && !move.isEnPassant) {
            const dot = document.createElement('div');
            dot.className = 'move-dot';
            sq.appendChild(dot);
          } else {
            const ring = document.createElement('div');
            ring.className = 'capture-ring';
            sq.appendChild(ring);
          }
        }
      }
    });
  }

  updateGameUI() {
    this.updateClocksUI();
    this.updateStatusText(this.game.statusText());
    this.updatePlayerCards();
    this.updateHistoryPanel();
    this.updateCapturedPieces();
  }

  updateClocksUI() {
    const timer = this.game.getTimer();
    const wEl = document.getElementById('clock-white');
    const bEl = document.getElementById('clock-black');

    wEl.textContent = ChessGame.Timer ? ChessGame.Timer.format(timer.whiteTime()) : this.formatTime(timer.whiteTime());
    bEl.textContent = ChessGame.Timer ? ChessGame.Timer.format(timer.blackTime()) : this.formatTime(timer.blackTime());

    wEl.classList.toggle('warning', timer.whiteTime() < 30);
    bEl.classList.toggle('warning', timer.blackTime() < 30);
  }

  formatTime(sec) {
    const total = Math.ceil(sec);
    const m = Math.floor(total / 60);
    const s = total % 60;
    return `${m.toString().padStart(2, '0')}:${s.toString().padStart(2, '0')}`;
  }

  updateStatusText(text) {
    document.getElementById('game-status').textContent = text;
  }

  updatePlayerCards() {
    const turn = this.game.turn();
    document.getElementById('card-white').classList.toggle('active-turn', turn === Color.White);
    document.getElementById('card-black').classList.toggle('active-turn', turn === Color.Black);
  }

  updateCapturedPieces() {
    const whiteCap = this.game.whiteCaptured();
    const blackCap = this.game.blackCaptured();

    let whiteScore = 0, blackScore = 0;
    whiteCap.forEach(p => whiteScore += p.value());
    blackCap.forEach(p => blackScore += p.value());

    const renderCaps = (elId, capArr, diff) => {
      const el = document.getElementById(elId);
      let html = capArr.map(p => p.glyph()).join('');
      if (diff > 0) {
        html += `<span class="material-diff">+${Math.floor(diff / 100)}</span>`;
      }
      el.innerHTML = html;
    };

    renderCaps('captured-white', whiteCap, whiteScore - blackScore);
    renderCaps('captured-black', blackCap, blackScore - whiteScore);
  }

  // Renders the move history generated from the C++ MoveLinkedList
  updateHistoryPanel() {
    const historyList = document.getElementById('history-list');
    const historyMoves = this.game.history(); // Walks MoveLinkedList nodes

    let html = '';
    for (let i = 0; i < historyMoves.length; i += 2) {
      const moveNum = Math.floor(i / 2) + 1;
      const wNotation = historyMoves[i] ? historyMoves[i].notation : '';
      const bNotation = historyMoves[i + 1] ? historyMoves[i + 1].notation : '';

      html += `
        <div class="move-row">
          <span class="move-num">${moveNum}.</span>
          <span class="white-m">${wNotation}</span>
          <span class="black-m">${bNotation}</span>
        </div>
      `;
    }
    historyList.innerHTML = html;
    historyList.scrollTop = historyList.scrollHeight;
  }

  handleGameOver() {
    this.sound.playGameOver();
    const statusText = this.game.statusText();
    this.updateStatusText(statusText);

    document.getElementById('gameover-title').textContent = statusText;
    document.getElementById('gameover-desc').textContent = "Game Finished";
    document.getElementById('gameover-modal').classList.add('active');
  }

  // --- Event Listeners Setup ---
  initEventListeners() {
    // Menu Mode Selection
    document.querySelectorAll('#mode-picker .opt-btn').forEach(btn => {
      btn.addEventListener('click', () => {
        document.querySelectorAll('#mode-picker .opt-btn').forEach(b => b.classList.remove('selected'));
        btn.classList.add('selected');
        const mode = parseInt(btn.dataset.mode, 10);
        document.getElementById('difficulty-section').style.display = (mode === GameMode.VsComputer) ? 'flex' : 'none';
      });
    });

    // Difficulty Selection
    document.querySelectorAll('#difficulty-picker .opt-btn').forEach(btn => {
      btn.addEventListener('click', () => {
        document.querySelectorAll('#difficulty-picker .opt-btn').forEach(b => b.classList.remove('selected'));
        btn.classList.add('selected');
      });
    });

    // Timer Selection
    document.querySelectorAll('#timer-picker .opt-btn').forEach(btn => {
      btn.addEventListener('click', () => {
        document.querySelectorAll('#timer-picker .opt-btn').forEach(b => b.classList.remove('selected'));
        btn.classList.add('selected');
      });
    });

    // Start Game Button
    document.getElementById('start-btn').addEventListener('click', () => {
      this.startGame();
    });

    // Game Screen Controls
    document.getElementById('undo-btn').addEventListener('click', () => this.handleUndo());
    document.getElementById('newgame-btn').addEventListener('click', () => {
      this.showConfirmDialog("Start New Game?", "Current game progress will be lost.", () => this.startGame());
    });
    document.getElementById('menu-btn').addEventListener('click', () => {
      this.showConfirmDialog("Return to Main Menu?", "Current game progress will be lost.", () => this.showScreen('menu-screen'));
    });

    // Theme Switcher
    const themes = ['wood', 'slate', 'emerald', 'midnight'];
    let themeIdx = 0;
    document.getElementById('theme-btn').addEventListener('click', () => {
      themeIdx = (themeIdx + 1) % themes.length;
      document.documentElement.setAttribute('data-board-theme', themes[themeIdx]);
    });

    // Sound FX Toggle
    document.getElementById('sound-btn').addEventListener('click', (e) => {
      this.sound.enabled = !this.sound.enabled;
      e.target.textContent = this.sound.enabled ? "🔊 Sound: ON" : "🔇 Sound: OFF";
    });

    // Pawn Promotion Modal Buttons
    document.querySelectorAll('.promo-btn').forEach(btn => {
      btn.addEventListener('click', () => {
        const promoType = parseInt(btn.dataset.piece, 10);
        document.getElementById('promo-modal').classList.remove('active');
        if (this.pendingPromoMove) {
          const m = this.pendingPromoMove;
          this.pendingPromoMove = null;
          this.commitMove(m.fromRow, m.fromCol, m.toRow, m.toCol, promoType);
        }
      });
    });

    // Game Over Buttons
    document.getElementById('rematch-btn').addEventListener('click', () => {
      document.getElementById('gameover-modal').classList.remove('active');
      this.startGame();
    });
    document.getElementById('gameover-menu-btn').addEventListener('click', () => {
      document.getElementById('gameover-modal').classList.remove('active');
      this.showScreen('menu-screen');
    });

    // Confirmation Modal Buttons
    document.getElementById('confirm-yes').addEventListener('click', () => {
      document.getElementById('confirm-modal').classList.remove('active');
      if (this.confirmAction) this.confirmAction();
    });
    document.getElementById('confirm-no').addEventListener('click', () => {
      document.getElementById('confirm-modal').classList.remove('active');
    });

    // Keyboard Shortcuts (U = Undo, Esc = Menu/Cancel, N = New Game)
    window.addEventListener('keydown', (e) => {
      if (document.querySelector('.modal-overlay.active')) {
        if (e.key === 'Escape') {
          document.querySelectorAll('.modal-overlay').forEach(m => m.classList.remove('active'));
        }
        return;
      }

      if (document.getElementById('game-screen').classList.contains('active')) {
        if (e.key === 'u' || e.key === 'U') {
          this.handleUndo();
        } else if (e.key === 'Escape') {
          this.showConfirmDialog("Return to Main Menu?", "Current game progress will be lost.", () => this.showScreen('menu-screen'));
        } else if (e.key === 'n' || e.key === 'N') {
          this.showConfirmDialog("Start New Game?", "Current game progress will be lost.", () => this.startGame());
        }
      }
    });
  }

  showScreen(screenId) {
    document.querySelectorAll('.screen').forEach(s => s.classList.remove('active'));
    document.getElementById(screenId).classList.add('active');
  }

  startGame() {
    const modeBtn = document.querySelector('#mode-picker .opt-btn.selected');
    const diffBtn = document.querySelector('#difficulty-picker .opt-btn.selected');
    const timerBtn = document.querySelector('#timer-picker .opt-btn.selected');

    const mode = parseInt(modeBtn.dataset.mode, 10);
    const diff = parseInt(diffBtn.dataset.diff, 10);
    const minutes = parseInt(timerBtn.dataset.minutes, 10);

    this.game.setMode(mode);
    this.game.setDifficulty(diff);
    this.game.setMinutes(minutes);
    this.game.newGame();

    document.getElementById('name-black').textContent = (mode === GameMode.VsComputer)
      ? `Computer (${diff === 0 ? 'Easy' : diff === 1 ? 'Medium' : 'Hard'})`
      : 'Black';

    this.resetSelection();
    this.showScreen('game-screen');
    this.updateBoardUI();
    this.updateGameUI();
  }

  handleUndo() {
    if (this.game.isOver() || this.aiThinking) return;
    const success = this.game.undo();
    if (success) {
      this.sound.playMove();
      this.resetSelection();
      this.updateBoardUI();
      this.updateGameUI();
    }
  }

  showConfirmDialog(title, desc, action) {
    document.getElementById('confirm-title').textContent = title;
    document.getElementById('confirm-desc').textContent = desc;
    this.confirmAction = action;
    document.getElementById('confirm-modal').classList.add('active');
  }
}

// Instantiate App when DOM is loaded
window.addEventListener('DOMContentLoaded', () => {
  new ChessApp();
});
