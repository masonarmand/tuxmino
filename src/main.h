#ifndef MAIN_H
#define MAIN_H

#include "engine.h"

void advanceLevel(int lineCount);
void declareGameOver(void);
void resetGame(void);

extern const int matrixHeight;
extern const int matrixWidth;

extern Sound pieceLockSound;
extern Sound landSound;
extern Sound cheerSound;
extern Sound lineClearSound;
extern Sound moveSound;
extern Sound preRotateSound;

#endif

