#pragma once
#include <string>
#include "Piece.h"

// ---------------------------------------------------------------------------
// Timer.h - simple chess clock for both players
// ---------------------------------------------------------------------------

class Timer {
public:
    explicit Timer(int minutes = 10);

    void setMinutes(int minutes);   // also resets both clocks
    void reset();
    void tick(Color activeSide, float deltaSeconds); // decrease the active clock

    float whiteTime() const { return whiteSeconds; }
    float blackTime() const { return blackSeconds; }
    void setTimes(float w, float b) { whiteSeconds = w; blackSeconds = b; }

    bool whiteFlagged() const { return whiteSeconds <= 0.f; }
    bool blackFlagged() const { return blackSeconds <= 0.f; }

    int minutes() const { return baseMinutes; }

    // Formats seconds as "MM:SS"
    static std::string format(float seconds);

private:
    int baseMinutes;
    float whiteSeconds;
    float blackSeconds;
};
