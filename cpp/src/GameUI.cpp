#include "GameUI.h"
#include <cstdio>
#include <sstream>

// ---------------------------------------------------------------------------
// Colour palette
// ---------------------------------------------------------------------------
static const sf::Color COL_BG          (24, 26, 33);
static const sf::Color COL_PANEL       (34, 37, 46);
static const sf::Color COL_PANEL_DARK  (28, 30, 38);
static const sf::Color COL_ACCENT      (99, 155, 255);
static const sf::Color COL_ACCENT_DARK (60, 100, 180);
static const sf::Color COL_TEXT        (232, 234, 240);
static const sf::Color COL_TEXT_DIM    (150, 156, 172);
static const sf::Color COL_LIGHT_SQ    (238, 226, 200);
static const sf::Color COL_DARK_SQ     (118, 91, 66);
static const sf::Color COL_SELECT      (246, 213, 92, 200);
static const sf::Color COL_LASTMOVE    (120, 190, 120, 130);
static const sf::Color COL_CHECK       (214, 72, 72, 190);
static const sf::Color COL_WHITE_PIECE (250, 250, 250);
static const sf::Color COL_BLACK_PIECE (26, 26, 30);
static const sf::Color COL_DANGER      (198, 70, 70);
static const sf::Color COL_GREEN       (80, 170, 110);

static const unsigned WIN_W = 1180;
static const unsigned WIN_H = 780;

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
GameUI::GameUI()
    : window(
        sf::VideoMode({WIN_W, WIN_H}),
        "Chess Master",
        sf::Style::Titlebar | sf::Style::Close
      ),
      fontLoaded(false),
      screen(Screen::Menu),
      boardX(40.f),
      boardY(90.f),
      squareSize(76.f),
      selectedRow(-1),
      selectedCol(-1),
      hasLastMove(false),
      menuDifficulty(Difficulty::Medium),
      menuMinutes(10),
      confirmNewGame(false),
      aiThinking(false)
{
    window.setFramerateLimit(60);

    const char* paths[] = {
        "assets/fonts/DejaVuSans.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/segoeui.ttf"
    };

    for (int i = 0; i < 3 && !fontLoaded; ++i) {
        if (font.openFromFile(paths[i])) {
            fontLoaded = true;
        }
    }
}

GameUI::~GameUI() {}

// ---------------------------------------------------------------------------
// Drawing helpers
// ---------------------------------------------------------------------------
void GameUI::drawText(
    const std::string& s,
    float x,
    float y,
    unsigned size,
    sf::Color color,
    bool bold,
    bool centered
) {
    if (!fontLoaded)
        return;

    // SFML 3 requires the font in the constructor
    sf::Text text(font, sf::String::fromUtf8(s.begin(), s.end()), size);

    text.setFillColor(color);

    if (bold) {
        text.setStyle(sf::Text::Bold);
    }

    if (centered) {
        sf::FloatRect b = text.getLocalBounds();

        text.setPosition({
            x - b.size.x / 2.f - b.position.x,
            y
        });
    }
    else {
        text.setPosition({x, y});
    }

    window.draw(text);
}

// ---------------------------------------------------------------------------
// Draw chess piece glyph
// ---------------------------------------------------------------------------
void GameUI::drawGlyph(
    unsigned int codepoint,
    float x,
    float y,
    unsigned size,
    sf::Color color
) {
    if (!fontLoaded)
        return;

    // SFML 3 uses char32_t for UTF-32 characters
    sf::String s(static_cast<char32_t>(codepoint));

    // SFML 3 constructor order:
    // Text(font, string, characterSize)
    sf::Text text(font, s, size);

    text.setFillColor(color);

    sf::FloatRect b = text.getLocalBounds();

    text.setPosition({
        x - b.size.x / 2.f - b.position.x,
        y - b.size.y / 2.f - b.position.y
    });

    window.draw(text);
}

// ---------------------------------------------------------------------------
// Draw panel
// ---------------------------------------------------------------------------
void GameUI::drawPanel(
    float x,
    float y,
    float w,
    float h,
    sf::Color fill
) {
    sf::RectangleShape r(sf::Vector2f(w, h));

    // SFML 3
    r.setPosition({x, y});

    r.setFillColor(fill);
    r.setOutlineThickness(1.f);
    r.setOutlineColor(sf::Color(58, 62, 74));

    window.draw(r);
}

// ---------------------------------------------------------------------------
// Draw button
// ---------------------------------------------------------------------------
void GameUI::drawButton(
    const Button& b,
    sf::Color fill,
    sf::Color textColor,
    unsigned size
) {
    // SFML 3 FloatRect:
    // position.x / position.y
    // size.x / size.y

    sf::RectangleShape r(
        sf::Vector2f(
            b.rect.size.x,
            b.rect.size.y
        )
    );

    r.setPosition({
        b.rect.position.x,
        b.rect.position.y
    });

    r.setFillColor(fill);
    r.setOutlineThickness(1.f);
    r.setOutlineColor(sf::Color(80, 86, 102));

    window.draw(r);

    drawText(
        b.label,
        b.rect.position.x + b.rect.size.x / 2.f,
        b.rect.position.y + b.rect.size.y / 2.f - size * 0.75f,
        size,
        textColor,
        true,
        true
    );
}

// ---------------------------------------------------------------------------
// Difficulty name
// ---------------------------------------------------------------------------
std::string GameUI::difficultyName() const {
    switch (menuDifficulty) {
        case Difficulty::Easy:
            return "EASY";

        case Difficulty::Medium:
            return "MEDIUM";

        default:
            return "HARD";
    }
}

// ---------------------------------------------------------------------------
// Menu buttons
// ---------------------------------------------------------------------------
std::vector<Button> GameUI::menuButtons() const {
    std::vector<Button> b;

    float w = 380.f;
    float h = 58.f;
    float x = (WIN_W - w) / 2.f;
    float y = 250.f;

    const char* labels[5] = {
        "PLAY VS COMPUTER",
        "MULTIPLAYER (2 PLAYERS)",
        "",
        "",
        "EXIT"
    };

    for (int i = 0; i < 5; ++i) {
        Button btn;

        btn.rect = sf::FloatRect(
            {x, y + i * 72.f},
            {w, h}
        );

        btn.label = labels[i];

        b.push_back(btn);
    }

    b[2].label = "DIFFICULTY:  " + difficultyName();

    char buf[48];

    std::snprintf(
        buf,
        sizeof(buf),
        "TIMER:  %d MINUTES",
        menuMinutes
    );

    b[3].label = buf;

    return b;
}

// ---------------------------------------------------------------------------
// Game buttons
// ---------------------------------------------------------------------------
std::vector<Button> GameUI::gameButtons() const {
    std::vector<Button> b;

    float y = boardY + 8 * squareSize + 20.f;

    float w = 180.f;
    float h = 46.f;

    const char* labels[3] = {
        "UNDO",
        "NEW GAME",
        "MAIN MENU"
    };

    for (int i = 0; i < 3; ++i) {
        Button btn;

        btn.rect = sf::FloatRect(
            {
                boardX + i * (w + 22.f),
                y
            },
            {
                w,
                h
            }
        );

        btn.label = labels[i];

        b.push_back(btn);
    }

    return b;
}

// ---------------------------------------------------------------------------
// Confirm buttons
// ---------------------------------------------------------------------------
std::vector<Button> GameUI::confirmButtons() const {
    std::vector<Button> b;

    Button yes;
    Button no;

    yes.rect = sf::FloatRect(
        {
            WIN_W / 2.f - 170.f,
            WIN_H / 2.f + 20.f
        },
        {
            150.f,
            48.f
        }
    );

    yes.label = "YES";

    no.rect = sf::FloatRect(
        {
            WIN_W / 2.f + 20.f,
            WIN_H / 2.f + 20.f
        },
        {
            150.f,
            48.f
        }
    );

    no.label = "NO";

    b.push_back(yes);
    b.push_back(no);

    return b;
}

// ---------------------------------------------------------------------------
// Main loop
// ---------------------------------------------------------------------------
void GameUI::run() {
    sf::Clock frameClock;

    while (window.isOpen()) {

        float dt = frameClock.restart().asSeconds();

        // ---------------------------------------------------------------
        // SFML 3 EVENT SYSTEM
        // ---------------------------------------------------------------
        while (auto event = window.pollEvent()) {

            // Window close
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            // -----------------------------------------------------------
            // Mouse button
            // -----------------------------------------------------------
            if (auto* mouse =
                    event->getIf<sf::Event::MouseButtonPressed>()) {

                if (mouse->button == sf::Mouse::Button::Left) {

                    handleMouseClick(
                        static_cast<float>(mouse->position.x),
                        static_cast<float>(mouse->position.y)
                    );
                }
            }

            // -----------------------------------------------------------
            // Keyboard
            // -----------------------------------------------------------
            if (auto* key =
                    event->getIf<sf::Event::KeyPressed>()) {

                if (key->code == sf::Keyboard::Key::Escape) {

                    if (confirmNewGame) {
                        confirmNewGame = false;
                    }
                    else if (screen == Screen::Playing) {
                        screen = Screen::Menu;
                    }
                }
                else if (
                    key->code == sf::Keyboard::Key::U &&
                    screen == Screen::Playing
                ) {
                    if (game.undo()) {
                        resetSelection();
                        hasLastMove = false;
                    }
                }
            }
        }

        // ---------------------------------------------------------------
        // Game update
        // ---------------------------------------------------------------
        if (screen == Screen::Playing && !confirmNewGame) {

            game.update(dt);

            // Computer move
            if (game.computerShouldMove()) {

                if (!aiThinking) {

                    aiThinking = true;
                    aiDelay.restart();
                }
                else if (
                    aiDelay.getElapsedTime().asSeconds() > 0.45f
                ) {

                    game.playComputerMove();

                    Move m;

                    if (game.lastMove(m)) {
                        lastMove = m;
                        hasLastMove = true;
                    }

                    aiThinking = false;
                }
            }
            else {
                aiThinking = false;
            }
        }

        // ---------------------------------------------------------------
        // Draw
        // ---------------------------------------------------------------
        window.clear(COL_BG);

        if (screen == Screen::Menu) {
            drawMenu();
        }
        else {
            drawGame();
        }

        if (confirmNewGame) {
            drawConfirmDialog();
        }

        window.display();
    }
}

// ---------------------------------------------------------------------------
// Input
// ---------------------------------------------------------------------------
void GameUI::handleMouseClick(float x, float y) {

    // Confirm dialog
    if (confirmNewGame) {

        std::vector<Button> b = confirmButtons();

        if (b[0].contains(x, y)) {

            game.newGame();

            resetSelection();

            hasLastMove = false;

            confirmNewGame = false;
        }
        else if (b[1].contains(x, y)) {

            confirmNewGame = false;
        }

        return;
    }

    if (screen == Screen::Menu) {
        handleMenuClick(x, y);
    }
    else {
        handleGameButtons(x, y);
        handleBoardClick(x, y);
    }
}

// ---------------------------------------------------------------------------
// Menu input
// ---------------------------------------------------------------------------
void GameUI::handleMenuClick(float x, float y) {

    std::vector<Button> b = menuButtons();

    if (b[0].contains(x, y)) {

        startGame(GameMode::VsComputer);
    }
    else if (b[1].contains(x, y)) {

        startGame(GameMode::Multiplayer);
    }
    else if (b[2].contains(x, y)) {

        menuDifficulty =
            static_cast<Difficulty>(
                (static_cast<int>(menuDifficulty) + 1) % 3
            );
    }
    else if (b[3].contains(x, y)) {

        const int options[5] = {
            1,
            3,
            5,
            10,
            15
        };

        int idx = 0;

        for (int i = 0; i < 5; ++i) {

            if (options[i] == menuMinutes) {
                idx = i;
            }
        }

        menuMinutes = options[(idx + 1) % 5];
    }
    else if (b[4].contains(x, y)) {

        window.close();
    }
}

// ---------------------------------------------------------------------------
// Start game
// ---------------------------------------------------------------------------
void GameUI::startGame(GameMode mode) {

    game.setMode(mode);

    game.setDifficulty(menuDifficulty);

    game.setMinutes(menuMinutes);

    game.newGame();

    resetSelection();

    hasLastMove = false;

    aiThinking = false;

    screen = Screen::Playing;
}

// ---------------------------------------------------------------------------
// Reset selection
// ---------------------------------------------------------------------------
void GameUI::resetSelection() {

    selectedRow = -1;
    selectedCol = -1;

    selectedMoves.clear();
}

// ---------------------------------------------------------------------------
// Game buttons
// ---------------------------------------------------------------------------
void GameUI::handleGameButtons(float x, float y) {

    std::vector<Button> b = gameButtons();

    // UNDO
    if (b[0].contains(x, y)) {

        if (game.undo()) {

            resetSelection();

            Move m;

            hasLastMove = game.lastMove(m);

            if (hasLastMove) {
                lastMove = m;
            }
        }
    }

    // NEW GAME
    else if (b[1].contains(x, y)) {

        confirmNewGame = true;
    }

    // MAIN MENU
    else if (b[2].contains(x, y)) {

        screen = Screen::Menu;
    }
}

// ---------------------------------------------------------------------------
// Board click
// ---------------------------------------------------------------------------
void GameUI::handleBoardClick(float x, float y) {

    if (game.isOver())
        return;

    if (
        game.getMode() == GameMode::VsComputer &&
        game.turn() != game.humanColor()
    ) {
        return;
    }

    if (x < boardX || y < boardY)
        return;

    int col =
        static_cast<int>(
            (x - boardX) / squareSize
        );

    int row =
        static_cast<int>(
            (y - boardY) / squareSize
        );

    if (
        row < 0 ||
        row > 7 ||
        col < 0 ||
        col > 7
    ) {
        return;
    }

    // ---------------------------------------------------------------
    // Second click
    // ---------------------------------------------------------------
    if (selectedRow >= 0) {

        for (size_t i = 0; i < selectedMoves.size(); ++i) {

            if (
                selectedMoves[i].toRow == row &&
                selectedMoves[i].toCol == col
            ) {

                if (
                    game.makeMove(
                        selectedRow,
                        selectedCol,
                        row,
                        col,
                        PieceType::Queen
                    )
                ) {

                    Move m;

                    if (game.lastMove(m)) {

                        lastMove = m;
                        hasLastMove = true;
                    }
                }

                resetSelection();

                return;
            }
        }
    }

    // ---------------------------------------------------------------
    // First click
    // ---------------------------------------------------------------
    const Piece& p =
        game.getBoard().at(row, col);

    if (
        !p.isEmpty() &&
        p.color == game.turn()
    ) {

        selectedRow = row;
        selectedCol = col;

        selectedMoves =
            game.legalMovesFrom(row, col);
    }
    else {

        resetSelection();
    }
}

// ---------------------------------------------------------------------------
// Main menu
// ---------------------------------------------------------------------------
void GameUI::drawMenu() {

    drawPanel(
        WIN_W / 2.f - 300.f,
        70.f,
        600.f,
        640.f,
        COL_PANEL
    );

    drawText(
        "CHESS MASTER",
        WIN_W / 2.f,
        110.f,
        54,
        COL_ACCENT,
        true,
        true
    );

    drawText(
        "A C++ / SFML desktop chess game",
        WIN_W / 2.f,
        178.f,
        18,
        COL_TEXT_DIM,
        false,
        true
    );

    std::vector<Button> b = menuButtons();

    for (size_t i = 0; i < b.size(); ++i) {

        sf::Color fill = COL_PANEL_DARK;
        sf::Color text = COL_TEXT;

        if (i == 0) {
            fill = COL_ACCENT_DARK;
        }

        if (i == 1) {
            fill = sf::Color(56, 62, 78);
        }

        if (i == 4) {
            fill = sf::Color(80, 40, 44);
        }

        drawButton(
            b[i],
            fill,
            text,
            20
        );
    }

    drawText(
        "Data structures: 2D Array (board)  -  Linked List (move history)  -  Stack (undo)",
        WIN_W / 2.f,
        655.f,
        15,
        COL_TEXT_DIM,
        false,
        true
    );
}

// ---------------------------------------------------------------------------
// Game screen
// ---------------------------------------------------------------------------
void GameUI::drawGame() {

    // Title bar
    drawPanel(
        0.f,
        0.f,
        static_cast<float>(WIN_W),
        66.f,
        COL_PANEL
    );

    drawText(
        "CHESS MASTER",
        40.f,
        16.f,
        30,
        COL_ACCENT,
        true
    );

    std::string modeText =
        (game.getMode() == GameMode::VsComputer)
        ?
        "Player vs Computer  (" + difficultyName() + ")"
        :
        "Multiplayer  -  2 Players";

    drawText(
        modeText,
        static_cast<float>(WIN_W) - 40.f,
        26.f,
        18,
        COL_TEXT_DIM,
        false,
        false
    );

    drawBoard();

    drawPieces();

    drawSidePanel();

    drawBottomBar();

    if (game.isOver()) {
        drawGameOverOverlay();
    }
}

// ---------------------------------------------------------------------------
// Board
// ---------------------------------------------------------------------------
void GameUI::drawBoard() {

    const Board& board = game.getBoard();

    // Board frame
    drawPanel(
        boardX - 10.f,
        boardY - 10.f,
        8 * squareSize + 20.f,
        8 * squareSize + 20.f,
        sf::Color(45, 38, 32)
    );

    int checkRow = -1;
    int checkCol = -1;

    if (game.inCheckNow()) {
        board.findKing(
            game.turn(),
            checkRow,
            checkCol
        );
    }

    // ---------------------------------------------------------------
    // Squares
    // ---------------------------------------------------------------
    for (int r = 0; r < 8; ++r) {

        for (int c = 0; c < 8; ++c) {

            sf::RectangleShape sq(
                sf::Vector2f(
                    squareSize,
                    squareSize
                )
            );

            sq.setPosition({
                boardX + c * squareSize,
                boardY + r * squareSize
            });

            sq.setFillColor(
                ((r + c) % 2 == 0)
                ?
                COL_LIGHT_SQ
                :
                COL_DARK_SQ
            );

            window.draw(sq);

            // Last move
            if (
                hasLastMove &&
                (
                    (
                        lastMove.fromRow == r &&
                        lastMove.fromCol == c
                    )
                    ||
                    (
                        lastMove.toRow == r &&
                        lastMove.toCol == c
                    )
                )
            ) {

                sq.setFillColor(COL_LASTMOVE);

                window.draw(sq);
            }

            // Selected square
            if (
                r == selectedRow &&
                c == selectedCol
            ) {

                sq.setFillColor(COL_SELECT);

                window.draw(sq);
            }

            // King in check
            if (
                r == checkRow &&
                c == checkCol
            ) {

                sq.setFillColor(COL_CHECK);

                window.draw(sq);
            }
        }
    }

    // ---------------------------------------------------------------
    // Legal move highlights
    // ---------------------------------------------------------------
    for (size_t i = 0; i < selectedMoves.size(); ++i) {

        const Move& m = selectedMoves[i];

        float cx =
            boardX +
            m.toCol * squareSize +
            squareSize / 2.f;

        float cy =
            boardY +
            m.toRow * squareSize +
            squareSize / 2.f;

        // Empty destination
        if (m.captured.isEmpty()) {

            sf::CircleShape dot(
                squareSize * 0.14f
            );

            dot.setFillColor(
                sf::Color(40, 90, 60, 170)
            );

            dot.setPosition({
                cx - dot.getRadius(),
                cy - dot.getRadius()
            });

            window.draw(dot);
        }

        // Capture destination
        else {

            sf::CircleShape ring(
                squareSize * 0.42f
            );

            ring.setFillColor(
                sf::Color::Transparent
            );

            ring.setOutlineThickness(5.f);

            ring.setOutlineColor(
                sf::Color(190, 60, 60, 200)
            );

            ring.setPosition({
                cx - ring.getRadius(),
                cy - ring.getRadius()
            });

            window.draw(ring);
        }
    }

    // ---------------------------------------------------------------
    // Coordinates
    // ---------------------------------------------------------------
    for (int i = 0; i < 8; ++i) {

        std::string file(
            1,
            static_cast<char>('a' + i)
        );

        std::string rank(
            1,
            static_cast<char>('8' - i)
        );

        drawText(
            file,
            boardX +
                i * squareSize +
                squareSize / 2.f,
            boardY +
                8 * squareSize +
                12.f,
            14,
            COL_TEXT_DIM,
            false,
            true
        );

        drawText(
            rank,
            boardX - 26.f,
            boardY +
                i * squareSize +
                squareSize / 2.f -
                10.f,
            14,
            COL_TEXT_DIM
        );
    }
}

// ---------------------------------------------------------------------------
// Chess pieces
// ---------------------------------------------------------------------------
void GameUI::drawPieces() {

    const Board& board = game.getBoard();

    for (int r = 0; r < 8; ++r) {

        for (int c = 0; c < 8; ++c) {

            const Piece& p =
                board.at(r, c);

            if (p.isEmpty())
                continue;

            float cx =
                boardX +
                c * squareSize +
                squareSize / 2.f;

            float cy =
                boardY +
                r * squareSize +
                squareSize / 2.f;

            // Shadow
            drawGlyph(
                p.glyph(),
                cx + 2.f,
                cy + 3.f,
                static_cast<unsigned>(
                    squareSize * 0.82f
                ),
                sf::Color(0, 0, 0, 70)
            );

            // Piece
            drawGlyph(
                p.glyph(),
                cx,
                cy,
                static_cast<unsigned>(
                    squareSize * 0.82f
                ),
                p.color == Color::White
                ?
                COL_WHITE_PIECE
                :
                COL_BLACK_PIECE
            );
        }
    }
}

// ---------------------------------------------------------------------------
// Side panel
// ---------------------------------------------------------------------------
void GameUI::drawSidePanel() {

    float px =
        boardX +
        8 * squareSize +
        40.f;

    float pw =
        static_cast<float>(WIN_W) -
        px -
        40.f;

    // ---------------------------------------------------------------
    // Status
    // ---------------------------------------------------------------
    drawPanel(
        px,
        90.f,
        pw,
        66.f,
        COL_PANEL
    );

    sf::Color statusColor =
        game.isOver()
        ?
        COL_DANGER
        :
        (
            game.inCheckNow()
            ?
            sf::Color(240, 180, 80)
            :
            COL_TEXT
        );

    drawText(
        "GAME STATUS",
        px + 16.f,
        100.f,
        13,
        COL_TEXT_DIM,
        true
    );

    drawText(
        game.statusText(),
        px + 16.f,
        120.f,
        21,
        statusColor,
        true
    );

    // ---------------------------------------------------------------
    // Timer
    // ---------------------------------------------------------------
    drawPanel(
        px,
        168.f,
        pw,
        92.f,
        COL_PANEL
    );

    drawText(
        "TIMER",
        px + 16.f,
        176.f,
        13,
        COL_TEXT_DIM,
        true
    );

    bool whiteActive =
        (game.turn() == Color::White) &&
        !game.isOver();

    drawText(
        "White",
        px + 16.f,
        200.f,
        18,
        whiteActive
        ?
        COL_ACCENT
        :
        COL_TEXT
    );

    drawText(
        Timer::format(
            game.getTimer().whiteTime()
        ),
        px + pw - 100.f,
        200.f,
        20,
        whiteActive
        ?
        COL_ACCENT
        :
        COL_TEXT,
        true
    );

    drawText(
        "Black",
        px + 16.f,
        228.f,
        18,
        (!whiteActive && !game.isOver())
        ?
        COL_ACCENT
        :
        COL_TEXT
    );

    drawText(
        Timer::format(
            game.getTimer().blackTime()
        ),
        px + pw - 100.f,
        228.f,
        20,
        (!whiteActive && !game.isOver())
        ?
        COL_ACCENT
        :
        COL_TEXT,
        true
    );

    // ---------------------------------------------------------------
    // Captured pieces
    // ---------------------------------------------------------------
    drawPanel(
        px,
        272.f,
        pw,
        104.f,
        COL_PANEL
    );

    drawText(
        "CAPTURED PIECES",
        px + 16.f,
        280.f,
        13,
        COL_TEXT_DIM,
        true
    );

    drawText(
        "White took:",
        px + 16.f,
        302.f,
        15,
        COL_TEXT_DIM
    );

    const std::vector<Piece>& wc =
        game.whiteCaptured();

    for (
        size_t i = 0;
        i < wc.size() && i < 16;
        ++i
    ) {

        drawGlyph(
            wc[i].glyph(),
            px + 130.f + i * 22.f,
            312.f,
            28,
            COL_BLACK_PIECE
        );
    }

    drawText(
        "Black took:",
        px + 16.f,
        338.f,
        15,
        COL_TEXT_DIM
    );

    const std::vector<Piece>& bc =
        game.blackCaptured();

    for (
        size_t i = 0;
        i < bc.size() && i < 16;
        ++i
    ) {

        drawGlyph(
            bc[i].glyph(),
            px + 130.f + i * 22.f,
            348.f,
            28,
            COL_WHITE_PIECE
        );
    }

    // ---------------------------------------------------------------
    // Move history
    // ---------------------------------------------------------------
    float hy = 388.f;

    float hh =
        boardY +
        8 * squareSize +
        20.f -
        hy +
        46.f;

    drawPanel(
        px,
        hy,
        pw,
        hh,
        COL_PANEL
    );

    drawText(
        "MOVE HISTORY",
        px + 16.f,
        hy + 8.f,
        13,
        COL_TEXT_DIM,
        true
    );

    // Linked List content
    std::vector<Move> moves =
        game.history();

    int totalPairs =
        (static_cast<int>(moves.size()) + 1) / 2;

    int visiblePairs =
        static_cast<int>(
            (hh - 40.f) / 22.f
        );

    int firstPair =
        totalPairs > visiblePairs
        ?
        totalPairs - visiblePairs
        :
        0;

    float y = hy + 32.f;

    for (
        int pair = firstPair;
        pair < totalPairs;
        ++pair
    ) {

        std::ostringstream line;

        line << (pair + 1) << ".";

        std::string num =
            line.str();

        drawText(
            num,
            px + 16.f,
            y,
            16,
            COL_TEXT_DIM
        );

        drawText(
            moves[pair * 2].notation,
            px + 60.f,
            y,
            16,
            COL_TEXT
        );

        if (
            pair * 2 + 1 <
            static_cast<int>(moves.size())
        ) {

            drawText(
                moves[pair * 2 + 1].notation,
                px + 150.f,
                y,
                16,
                COL_TEXT
            );
        }

        y += 22.f;
    }

    if (moves.empty()) {

        drawText(
            "No moves yet",
            px + 16.f,
            hy + 32.f,
            15,
            COL_TEXT_DIM
        );
    }
}

// ---------------------------------------------------------------------------
// Bottom buttons
// ---------------------------------------------------------------------------
void GameUI::drawBottomBar() {

    std::vector<Button> b =
        gameButtons();

    drawButton(
        b[0],
        sf::Color(70, 76, 96),
        COL_TEXT,
        18
    );

    drawButton(
        b[1],
        COL_ACCENT_DARK,
        COL_TEXT,
        18
    );

    drawButton(
        b[2],
        sf::Color(70, 60, 60),
        COL_TEXT,
        18
    );
}

// ---------------------------------------------------------------------------
// Game over overlay
// ---------------------------------------------------------------------------
void GameUI::drawGameOverOverlay() {

    sf::RectangleShape shade(
        sf::Vector2f(
            8 * squareSize,
            8 * squareSize
        )
    );

    shade.setPosition({
        boardX,
        boardY
    });

    shade.setFillColor(
        sf::Color(10, 12, 18, 200)
    );

    window.draw(shade);

    float cx =
        boardX +
        4 * squareSize;

    drawText(
        "GAME OVER",
        cx,
        boardY + 200.f,
        46,
        COL_ACCENT,
        true,
        true
    );

    drawText(
        game.statusText(),
        cx,
        boardY + 270.f,
        26,
        COL_TEXT,
        true,
        true
    );

    drawText(
        "Press NEW GAME to play again, or MAIN MENU to change mode.",
        cx,
        boardY + 320.f,
        16,
        COL_TEXT_DIM,
        false,
        true
    );
}

// ---------------------------------------------------------------------------
// Confirm dialog
// ---------------------------------------------------------------------------
void GameUI::drawConfirmDialog() {

    sf::RectangleShape shade(
        sf::Vector2f(
            static_cast<float>(WIN_W),
            static_cast<float>(WIN_H)
        )
    );

    shade.setFillColor(
        sf::Color(8, 10, 14, 190)
    );

    window.draw(shade);

    drawPanel(
        WIN_W / 2.f - 230.f,
        WIN_H / 2.f - 100.f,
        460.f,
        200.f,
        COL_PANEL
    );

    drawText(
        "Start a new game?",
        WIN_W / 2.f,
        WIN_H / 2.f - 70.f,
        26,
        COL_TEXT,
        true,
        true
    );

    drawText(
        "The current game will be lost.",
        WIN_W / 2.f,
        WIN_H / 2.f - 30.f,
        16,
        COL_TEXT_DIM,
        false,
        true
    );

    std::vector<Button> b =
        confirmButtons();

    drawButton(
        b[0],
        COL_GREEN,
        COL_TEXT,
        18
    );

    drawButton(
        b[1],
        sf::Color(80, 44, 44),
        COL_TEXT,
        18
    );
}