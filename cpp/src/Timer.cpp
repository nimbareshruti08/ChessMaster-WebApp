#include "Timer.h"
#include <cstdio>

Timer::Timer(int minutes) : baseMinutes(minutes) { reset(); }

void Timer::setMinutes(int minutes) {
    baseMinutes = minutes;
    reset();
}

void Timer::reset() {
    whiteSeconds = blackSeconds = static_cast<float>(baseMinutes * 60);
}

void Timer::tick(Color activeSide, float deltaSeconds) {
    if (activeSide == Color::White) {
        whiteSeconds -= deltaSeconds;
        if (whiteSeconds < 0.f) whiteSeconds = 0.f;
    } else if (activeSide == Color::Black) {
        blackSeconds -= deltaSeconds;
        if (blackSeconds < 0.f) blackSeconds = 0.f;
    }
}

std::string Timer::format(float seconds) {
    if (seconds < 0.f) seconds = 0.f;
    int total = static_cast<int>(seconds + 0.999f);
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%02d:%02d", total / 60, total % 60);
    return std::string(buf);
}
