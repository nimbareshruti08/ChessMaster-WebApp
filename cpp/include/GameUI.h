#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include "ChessGame.h"

// ---------------------------------------------------------------------------
// GameUI.h - SFML window, main menu, board rendering and mouse handling
// ---------------------------------------------------------------------------

enum class Screen { Menu, Playing };

// A simple clickable rectangle with a text label.
struct Button {
    sf::FloatRect rect;
    std::string label;

    bool contains(float x, float y) const {
        return rect.contains({x, y});
    }
};

class GameUI {
public:
    GameUI();
    ~GameUI();

    void run();

private:
    sf::RenderWindow window;
    sf::Font font;
    bool fontLoaded;

    ChessGame game;
    Screen screen;

    // Board geometry
    float boardX, boardY, squareSize;

    // Selection / highlighting
    int selectedRow, selectedCol;
    std::vector<Move> selectedMoves;
    Move lastMove;
    bool hasLastMove;

    // Menu configuration
    Difficulty menuDifficulty;
    int menuMinutes;

    // Dialogs
    bool confirmNewGame;

    // Computer move pacing
    sf::Clock aiDelay;
    bool aiThinking;

    // Drawing helpers
    void drawMenu();
    void drawGame();
    void drawBoard();
    void drawPieces();
    void drawSidePanel();
    void drawBottomBar();
    void drawGameOverOverlay();
    void drawConfirmDialog();

    void drawText(const std::string& s,
                  float x,
                  float y,
                  unsigned size,
                  sf::Color color,
                  bool bold = false,
                  bool centered = false);

    void drawGlyph(unsigned int codepoint,
                   float x,
                   float y,
                   unsigned size,
                   sf::Color color);

    void drawPanel(float x,
                   float y,
                   float w,
                   float h,
                   sf::Color fill);

    void drawButton(const Button& b,
                    sf::Color fill,
                    sf::Color textColor,
                    unsigned size = 20);

    // Input
    void handleMouseClick(float x, float y);
    void handleMenuClick(float x, float y);
    void handleBoardClick(float x, float y);
    void handleGameButtons(float x, float y);

    void startGame(GameMode mode);
    void resetSelection();

    // Button layouts
    std::vector<Button> menuButtons() const;
    std::vector<Button> gameButtons() const;
    std::vector<Button> confirmButtons() const;

    std::string difficultyName() const;
};