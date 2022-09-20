#include <raylib.h>
#include "timer.h"

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


double GetElapsed(Timer timer) {
    double elapsed = 0;
    if (timer.startTime == 0) {
      elapsed = 0;
    } 
    else {
    elapsed = GetTime() - timer.startTime;
    }
    return elapsed;
}

