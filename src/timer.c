/*
 * File: timer.c
 * -------------
 * functions for Timer and GameTimer structs
 * Timer is used for various timings such as lock delay
 * GameTimer is used to measure the duration of a game
 *
 * Original Author: Mason Armand
 * Contributors:
 * Date Created: Sep 14, 2022
 * Last Modified: Feb 6, 2023
 */

#include "tuxmino.h"

void startTimer(Timer *timer, double lifetime) {
    timer->startTime = GetTime();
    timer->lifeTime = lifetime;
}

void resetTimer(Timer *timer) {
    timer->startTime = 0;
    timer->lifeTime = 0;
}

bool TimerDone(Timer timer) {
    return GetTime() - timer.startTime >= timer.lifeTime;
}

void startGameTimer(GameTimer *timer) {
    timer->startTime = GetTime();
}

void resetGameTimer(GameTimer *timer) {
    timer->startTime = 0;
}

void togglePauseGameTimer(GameTimer *timer) {
    if (!timer->paused) {
        timer->pauseTime = GetGameTimerElapsed(*timer);
        timer->paused = true;
    }
    else {
        timer->paused = false;
        timer->startTime = GetTime();
    }
}

double GetGameTimerElapsed(GameTimer timer) {
    double elapsed = 0;

    if (timer.startTime == 0)
        elapsed = 0;
    else if (!timer.paused)
        elapsed = (GetTime() - timer.startTime) + timer.pauseTime;
    else if (timer.paused)
        elapsed = timer.pauseTime;

    return elapsed;
}


double GetElapsed(Timer timer) {
    double elapsed = 0;
    if (timer.startTime == 0)
      elapsed = 0;
    else
        elapsed = GetTime() - timer.startTime;

    return elapsed;
}

