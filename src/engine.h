#ifndef ENGINE_H
#define ENGINE_H

// includes
#include <raylib.h>
#include "timer.h" // timer struct and functions, not to be confused with time.h

// ---------------------
// defined structs
// ---------------------

typedef struct {
    int type;
    bool visible;
    Texture2D tileset;
} Block;

typedef struct {
    Vector2 position;
    Texture2D tileset;
    int pieceIndex;
    int rotIndex;
} Piece;


// ---------------------
// prototyped functions
// ---------------------

// engine.c functions

// called in update() in main.c: 
void spawnQueuedPiece(Piece* activePiece, float tickSpeed, Block** playField);
void checkIfAtBottom(Piece* activePiece, Block** playField, float lineClearSpeed, float lockDelay, float appearanceDelay);
void processInput(Piece* activePiece, Block** playField, float delayedAutoShift, float autoRepeatRate, float lockDelay);
void moveDown(Piece* activePiece, Block** playField, float tickSpeed, float lineClearSpeed, float lockDelay, float appearanceDelay, bool gravity20G);

bool processMenuInput(int* gameType);
bool processPauseMenuInput(int* idxOption);

bool canDrawPiece();

Block** initMatrix(int width, int height, Texture blockTileset);
void generateInitialPreview(Piece* activePiece);

void fillRow(int row, int typeID, Block** playField);
Vector2 getFinalPos(Piece* activePiece, Block** playField);

void unloadMatrix(Block** matrix, int height);
void setPiece(int piece[4][4]);


extern int upcomingPieces[3];
extern int heldPiece;
#endif 
