#ifndef TIMER_H
#define TIMER_H

typedef struct {
  double startTime;
  double lifeTime;
} Timer;

// timer.c functions
void startTimer(Timer *timer, double lifetime);
void resetTimer(Timer *timer);
bool TimerDone(Timer timer);
double GetElapsed(Timer timer);


#endif
