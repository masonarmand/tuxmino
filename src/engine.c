/*
 * File: engine.c
 * --------------
 * This file contains the core mechanics of the game. Everything such as
 * wallkicks, rotation, movement, gameplay, etc is defined in this file.
 *
 * Original Author: Mason Armand
 * Contributors:
 * Date Created: Sep 14, 2022
 * Last Modified: Feb 9, 2023
 */
#include "tuxmino.h"
#include <stdlib.h> // required for malloc(), free()
#include <stdio.h>

// globally defined variables
int upcomingPieces[3];
int queuedLines[4] = {-1,-1,-1,-1};
int heldPiece = -1;

// internal functions
void shiftBag(int newPiece);
bool bagContains(int piece);
bool centerColumnCheck(Piece* activePiece, PlayField playField);
bool isValidRotation(int piece[4][4], Piece activePiece, PlayField playField);
int getRandomPiece(SpeedSettings rule);
void updatePreview(SpeedSettings rule);
void checkIfGameOver(Piece* activePiece, PlayField playField);
void queuePiece(Piece* activePiece, PlayField playField);
void holdSwap(Piece* activePiece, SpeedSettings rule, PlayField playField);
void moveRowsDown(int blankRow, PlayField playField, bool playSound);
void queueLineClear(int row);
void checkForLineClear(PlayField playField, SpeedSettings timings);
void resetLockDelay(SpeedSettings timings);
bool isTouchingStack(Piece* activePiece, PlayField playField);
bool isTouchingFloor(Piece* activePiece, PlayField playField);
void performWallKick(Piece* activePiece, SpeedSettings rule, PlayField playField, int rotation);
void softDrop(Piece* activePiece, PlayField playField, SpeedSettings* timings);
void lockIfFloorKicked(Piece* activePiece, PlayField playField, SpeedSettings timings);
void movePiece(Piece* activePiece, PlayField playField, int dir, SpeedSettings timings);
void moveIgnoreDAS(Piece* activePiece, PlayField playField, int dir, SpeedSettings timings);
void rotateRight(Piece* activePiece, SpeedSettings rule, PlayField playField);
void rotateLeft(Piece* activePiece, SpeedSettings rule, PlayField playField);
void makePlayFieldCopy(PlayField playField);

static Timer keyRepeatRateTimer;
static Timer keyRepeatDelayTimer;
static Timer tickTimer;
static Timer pieceLockDelayTimer;
static Timer lineClearDelayTimer;
static Timer appearanceDelayTimer;
static Timer flashTimer;

static int recentPieces[4] = {4,4,5,5};
static int queuedPiece = -1;
static int lockDelayResetCount = 1;

//TODO static int guidelineBag[7];

static bool canHold = true;
static bool canRotate = true;
static bool canSoftDrop = true;
static bool isLinesQueued = false;

/*
 * For extra T and I floor kicks. Used to determine if the piece should insta
 * lock after kicking
 */
static bool floorKicked = false;
static bool rotatedAfterKick = false;

static const int worldKicks[4][5][2] =
{
    { {0, 0}, {-1, 0}, {-1, 1}, {0,-2}, {-1,-2}, },
    { {0, 0}, {1,  0}, {1, -1}, {0, 2}, {1,  2}, },
    { {0, 0}, {1,  0}, {1,  1}, {0,-2}, {1, -2}, },
    { {0, 0}, {-1, 0}, {-1,-1}, {0, 2}, {-1, 2}, },
};

static const int worldIKicks[8][5][2] =
{
    { { 0, 0 }, {-2, 0 }, { 1, 0 }, { 1,2 }, {-2, -1 }, },
    { { 0, 0 }, { 2, 0 }, {-1, 0 }, { 2,1 }, {-1, -2 }, },
    { { 0, 0 }, {-1, 0 }, { 2, 0 }, {-1,2 }, { 2, -1 }, },
    { { 0, 0 }, {-2, 0 }, { 1, 0 }, {-2,1 }, { 1, -1 }, },
    { { 0, 0 }, { 2, 0 }, {-1, 0 }, { 2,1 }, {-1, -1 }, },
    { { 0, 0 }, { 1, 0 }, {-2, 0 }, { 1,2 }, {-2, -1 }, },
    { { 0, 0 }, {-2, 0 }, { 1, 0 }, {-2,1 }, { 1, -2 }, },
    { { 0, 0 }, { 2, 0 }, {-1, 0 }, {-1,2 }, { 2, -1 }, },
};

static const int classicKicks[2][2] = { {1, 0}, {-1, 0} };
static const int classicIKicks[5][2] =
{
    { 2, 0 },
    {-1, 0 },
    { 1, 0 },
    { 0, 1 },
    { 0, 2 }
};

static int rotationDir = 0; // 0 = left, 1 = right

void currentPieceToMatrix(Piece* activePiece, PlayField playField, SpeedSettings timings);


bool canDrawPiece(void) {
    if (GetElapsed(flashTimer) != 0) {
        return true;
    }
    return (TimerDone(appearanceDelayTimer) && TimerDone(lineClearDelayTimer));
}


bool canDrawFlash(void) {
    return !TimerDone(flashTimer);
}


/*
 * Adds a new piece into the first slot of the bag and then moves all other
 * pieces over.
 */
void shiftBag(int newPiece) {
    // create a new array that has the same values as recentPieces, but moved
    // over by 1 index.
    int newBag[4];
    for (unsigned int i = 0; i < 3; i++) {
        newBag[i+1] = recentPieces[i];
    }
    newBag[0] = newPiece;

    for (unsigned int i = 0; i < 4; i++) {
        recentPieces[i] = newBag[i];
    }
}


/*
 * Loops through the bag to see if a certain piece is in it or not.
 */
bool bagContains(int piece) {
    bool hasPiece = false;
    for (unsigned int i = 0; i < 4; i++) {
        if (recentPieces[i] == piece) {
            hasPiece = true;
        }
    }
    return hasPiece;
}


/*
 * When doing a wall kick, the piece must only be kicked if a block is not
 * occupying a space in the center column of the piece.
 * For more details on how this algorithm works see:
 * https://harddrop.com/wiki/Ars#Wall_kicks
 */
bool centerColumnCheck(Piece* activePiece, PlayField playField) {
    bool valid = false;

    // only the L, J, and T pieces have the center column check
    if ((activePiece->rotIndex == 0 || activePiece->rotIndex == 2) &&
        (activePiece->pieceIndex == 1 || // T piece
         activePiece->pieceIndex == 2 || // L piece
         activePiece->pieceIndex == 3))  // J piece
    {
        for (unsigned int y = 0; y < 3; y++) {
            for (unsigned int x = 0; x < 3; x++) {
                int posX = x + activePiece->position.x;
                int posY = y + activePiece->position.y;
                if (x != 1 && playField.matrix[posY][posX].type != 0) {
                    valid = true;
                    goto exitCenterColumnCheck;
                }
                else if (x == 1 && playField.matrix[posY][posX].type != 0) {
                    valid = false;
                    goto exitCenterColumnCheck;
                }
                else {
                    valid = true;
                }
            }
        }
    }
    else {
        valid = true;
    }

    exitCenterColumnCheck:
    return valid;
}


/*
 * This function checks whether or not the piece is in a valid position.
 * Invalid positions mean the piece is outside the playfield or is inside
 * another block.
 */
bool isValidRotation(int piece[4][4], Piece activePiece, PlayField playField) {
    bool valid = false;

    for (unsigned int y = 0; y < 4; y++) {
        for (unsigned int x = 0; x < 4; x++) {
            if (piece[y][x] != 0) {
                int xPos = activePiece.position.x;
                int yPos = activePiece.position.y;
                if (xPos + x > playField.width - 1 ||
                    yPos + y > playField.height - 1)
                {
                    valid = false;
                    goto exitRotationLoop;
                }
                if (playField.matrix[y+yPos][x+xPos].type == 0) {
                    valid = true;
                }
                else {
                    valid = false;
                    goto exitRotationLoop;
                }
            }
        }
    }
    exitRotationLoop:
    return valid;
}


/*
 * Generates a random piece index based on what settings are active.
 */
int getRandomPiece(SpeedSettings rule)
{
    unsigned int attempts = 0;
    unsigned int randomPiece;

    // Attempt to generate a piece that is not in the bag.
    // This will be attempted a certain number of times
    // according to the rule settings.
    while (attempts != rule.bagRetry) {
        randomPiece = GetRandomValue(0, 6);

        if (!bagContains(randomPiece)) {
            shiftBag(randomPiece);
            break;
        }
        attempts ++;
    }

    return randomPiece;
}


/*
 * Update the displayed next pieces.
 */
void updatePreview(SpeedSettings rule) {
    int newArr[3];
    for (unsigned int i = 3; i > 0; i--) {
        newArr[i-1] = upcomingPieces[i];
    }

    for (unsigned int i = 0; i < 3; i++) {
        upcomingPieces[i] = newArr[i];
    }

   upcomingPieces[2] = getRandomPiece(rule);
}


/*
 * This function checks to see if the current piece is inside of another block.
 */
void checkIfGameOver(Piece* activePiece, PlayField playField) {
    bool gameOver = false;
    int piece = activePiece->pieceIndex;
    int rotation = activePiece->rotIndex;

    for (unsigned int y = 0; y < 4; y++) {
        for (unsigned int x = 0; x < 4; x++) {
            int xPos = activePiece->position.x + x;
            int yPos = activePiece->position.y + y;

            if (pieces[piece][rotation][y][x] != 0 &&
                playField.matrix[yPos][xPos].type != 0)
            {
                gameOver = true;
            }
        }
    }

    //TODO remove declareGameOver, main() functions should not be called
    if (gameOver == true) {
        declareGameOver();
    }
}


/*
 * Queues a piece for spawning. This is used for things such as appearance
 * delay so that the piece does not appear immediately.
 */
void queuePiece(Piece* activePiece, PlayField playField) {
    // center the piece
    activePiece->position.x = (int)(playField.width / 2) - 2;
    activePiece->position.y = 1.0f;

    queuedPiece = upcomingPieces[0];
    activePiece->pieceIndex = upcomingPieces[0];
    activePiece->rotIndex = 0;
}


/*
 * Swaps the current piece with the held piece.
 */
void holdSwap(Piece* activePiece, SpeedSettings rule, PlayField playField) {
    if (!canHold || !rule.holdEnabled) {
        return;
    }

    int currentPiece = activePiece->pieceIndex;
    if (heldPiece == -1) {
        activePiece->pieceIndex = upcomingPieces[0];
        updatePreview(rule);
    }
    else {
        activePiece->pieceIndex = heldPiece;
    }
    canHold = false;
    heldPiece = currentPiece;
    activePiece->position.x = (int)(playField.width / 2)-2;
    activePiece->position.y = 1.0f;
    activePiece->rotIndex = 0;
}


/*
 * spawns the queuedPiece if all delay requirements are met.
 * pre-rotation and pre-hold is also done here.
 */
void spawnQueuedPiece(Piece* activePiece, PlayField playField, SpeedSettings timings)
{
    if (queuedPiece == -1 ||
        !TimerDone(appearanceDelayTimer) ||
        !TimerDone(lineClearDelayTimer))
    {
        return;
    }

    resetTimer(&flashTimer);
    resetTimer(&pieceLockDelayTimer);

    canSoftDrop = false;
    lockDelayResetCount = 0;
    activePiece->rotIndex = 0;
    activePiece->locked = false;
    floorKicked = false;
    rotatedAfterKick = false;

    canRotate = true;
    canHold = true;

    startTimer(&tickTimer, timings.tickSpeed);
    activePiece->pieceIndex = queuedPiece;
    queuedPiece = -1;
    updatePreview(timings);

    // pre hold
    if (IsKeyDown(config.keyBinds.swapHold)) {
        holdSwap(activePiece, timings, playField);
    }

    activePiece->position.x = (int)(playField.width / 2)-2;
    activePiece->position.y = 1.0f;

    int idx = activePiece->pieceIndex;
    // Pre rotation
    if ((IsKeyDown(config.keyBinds.ccw) ||
         IsKeyDown(config.keyBinds.ccwAlt)) &&
         isValidRotation(pieces[idx][3], *activePiece, playField))
    {
        activePiece->rotIndex = 3;
        PlaySound(preRotateSound);
    }
    else if (IsKeyDown(config.keyBinds.cw) &&
             isValidRotation(pieces[idx][1], *activePiece, playField))
    {
        activePiece->rotIndex = 1;
        PlaySound(preRotateSound);
    }

    PlaySound(pieceSounds[upcomingPieces[0]]);

    resetTimer(&appearanceDelayTimer);
    resetTimer(&lineClearDelayTimer);

    checkIfGameOver(activePiece, playField);
}


/*
 * This will generate random pieces for every next piece slot.
 * This should be called the first time you begin the game.
 */
void generateInitialPreview(Piece* activePiece, PlayField playField, SpeedSettings rule)
{
    upcomingPieces[0] = getRandomPiece(rule);

    /* first piece cant be S or Z */
    while (upcomingPieces[0] == 4 || upcomingPieces[0] == 5) {
        upcomingPieces[0] = getRandomPiece(rule);
    }

    for (unsigned int i = 1; i < 3; i++) {
        upcomingPieces[i] = getRandomPiece(rule);
    }

    queuePiece(activePiece, playField);
}


/*
 * Fills a row in the playfield with a certain block type.
 */
void fillRow(int row, int typeID, PlayField playField)
{
    for (unsigned int x = 0; x < playField.width; x++) {
        playField.matrix[row][x].type = typeID;
    }
}


/*
 * moves every row down according to a blank row. Used for moving the playfield
 * down after line clears.
 */
void moveRowsDown(int blankRow, PlayField playField, bool playSound)
{
    for (unsigned int y = blankRow; y > 0; y--) {
        for (unsigned int x = 0; x < playField.width; x++) {
            playField.matrix[y][x] = playField.matrix[y - 1][x];
        }
    }
    if (playSound)
        PlaySound(boardFall);
}


/*
 * queue a line clear, this is so that lines can hover in place
 * while the line clear animation plays.
 */
void queueLineClear(int row)
{
    for (unsigned int i = 0; i < 4; i++) {
        if (queuedLines[i] == -1) {
            queuedLines[i] = row;
            isLinesQueued = true;
            break;
        }
    }
}


/*
 * Calls moveRowsDown() for each line that is queued.
 */
void moveQueuedLinesDown(PlayField playField)
{
    unsigned int lastLineIdx;

    for (unsigned int i = 0; i < 4; i++) {
        if (queuedLines[i] == -1)
            break;
        else
            lastLineIdx = i;
    }

    if (!isLinesQueued || !TimerDone(lineClearDelayTimer)) {
        return;
    }

    for (unsigned int i = 0; i < 4; i++) {
        if (queuedLines[i] != -1) {
            bool playSound = (lastLineIdx == i);
            moveRowsDown(queuedLines[i], playField, playSound);
            queuedLines[i] = -1;
            isLinesQueued = false;
        }
    }
    resetTimer(&lineClearDelayTimer);
}


void makePlayFieldCopy(PlayField playField)
{
    for (unsigned int y = 0; y < playField.height; y++) {
        for (unsigned int x = 0; x < playField.width; x++) {
            playField.copy[y][x] = playField.matrix[y][x];
        }
    }
}

/*
 * Check to see if there are any rows that are completely full.
 * If so, the row will be completely filled with empty blocks
 * and then it will be queued.
 */
void checkForLineClear(PlayField playField, SpeedSettings timings)
{
    int lineCount = 0;
    bool copyBoard = true;
    for (unsigned int y = 0; y < playField.height; y++) {
        bool isFull = true;

        for (unsigned int x = 0; x < playField.width; x++) {
            if (playField.matrix[y][x].type == 0) {
                isFull = false;
            }
        }

        if (isFull && copyBoard) {
            makePlayFieldCopy(playField);
            copyBoard = false;
        }
        if (isFull) {
            canRotate = false;
            fillRow(y, 0, playField);
            startTimer(&lineClearDelayTimer, timings.lineClearSpeed);
            startTimer(&appearanceDelayTimer, timings.appearanceDelay + timings.lineClearSpeed);
            lineCount++;
            queueLineClear(y);
        }
    }

    if (lineCount == 4)
        PlaySound(cheerSound);
    else if (lineCount >= 1)
        PlaySound(lineClearSound);

    advanceLevel(lineCount);
}


/*
 * This function copies the current piece to the playfield depending
 * on its position.
 */
void currentPieceToMatrix(Piece* activePiece, PlayField playField, SpeedSettings timings)
{
    canHold = false;
    canRotate = false;
    for (unsigned int y = 0; y < 4; y++) {
        for (unsigned int x = 0; x < 4; x++) {
            if (pieces[activePiece->pieceIndex][activePiece->rotIndex][y][x] == 0) {
                continue;
            }

            int xPos = activePiece->position.x;
            int yPos = activePiece->position.y;

            playField.matrix[y+yPos][x+xPos].type = pieces[activePiece->pieceIndex][activePiece->rotIndex][y][x];
            playField.matrix[y+yPos][x+xPos].tileset = activePiece->currentTileset;
        }
    }

    PlaySound(pieceLockSound);
    checkForLineClear(playField, timings);
    queuePiece(activePiece, playField);
}


/*
 * Reset the lock delay depending on the rotation rule.
 * lock delay will only be reset if rotationRule is equal to WORLD rule.
 */
void resetLockDelay(SpeedSettings timings)
{
    int maxReset;
    if (rotationRule == WORLD) {
        maxReset = 8;
    }
    else {
        maxReset = 0;
    }
    if (lockDelayResetCount < maxReset) {
        resetTimer(&pieceLockDelayTimer);
        startTimer(&pieceLockDelayTimer, timings.lockDelay);
        lockDelayResetCount ++;
    }
}


/*
 * Checks to see if there is any blocks colliding with the current piece
 * in the direction it is moving.
 */
bool canMove(Vector2 position, Piece* activePiece, PlayField playField, int dirX, int dirY)
{
    bool move = false;

    for (unsigned int y = 0; y < 4; y++) {
        for (unsigned int x = 0; x < 4; x++) {
            if (pieces[activePiece->pieceIndex][activePiece->rotIndex][y][x] == 0) {
                continue;
            }

            unsigned int xPos = x + position.x + dirX;
            unsigned int yPos = y + position.y + dirY;

            if (xPos >= playField.width || yPos >= playField.height) {
                move = false;
                return move;
            }

            int dirBlock = playField.matrix[yPos][xPos].type;

            if (dirBlock == 0) {
                move = true;
            }
            else if (dirBlock != 0 &&
                     pieces[activePiece->pieceIndex][activePiece->rotIndex][y][x] != 0)
            {
                move = false;
                return move;
            }
        }
    }

    return move;
}


/*
 * Checks to see if the current piece is directly next to any blocks.
 * Useful for determining if certain pieces are allowed to kick.
 */
bool isTouchingStack(Piece* activePiece, PlayField playField)
{
    bool touchingStack = false;

    for (unsigned int y = 0; y < 4; y++) {
        for (unsigned int x = 0; x < 4; x++) {
            unsigned int xPos = activePiece->position.x + x;
            unsigned int yPos = activePiece->position.y + y;
            if (pieces[activePiece->pieceIndex][activePiece->rotIndex][y][x] == 0) {
                continue;
            }

            if (xPos == 0 ||
                playField.matrix[yPos][xPos - 1].type != 0 ||
                xPos + 1 > playField.height - 3 ||
                playField.matrix[yPos][xPos + 1].type != 0)
            {
                touchingStack = true;
                return touchingStack;
            }
        }
    }

    return touchingStack;
}


/*
 * Checks to see if the current piece is directly next to any blocks.
 * Useful for determining if certain pieces are allowed to kick.
 */
bool isTouchingFloor(Piece* activePiece, PlayField playField)
{
    bool touchingFloor = false;

    for (unsigned int y = 0; y < 4; y++) {
        for (unsigned int x = 0; x < 4; x++) {
            if (pieces[activePiece->pieceIndex][activePiece->rotIndex][y][x] == 0) {
                continue;
            }

            unsigned int xPos = activePiece->position.x + x;
            unsigned int yPos = activePiece->position.y + y;

            if (yPos + 1 >= playField.height - 1 || playField.matrix[yPos + 1][xPos].type != 0) {
                touchingFloor = true;
                return touchingFloor;
            }
        }
    }

    return touchingFloor;
}


void performWallKick(Piece* activePiece, SpeedSettings rule, PlayField playField, int rotation)
{
    int currentRot = activePiece->rotIndex;

    // classic rule kicks (ars)
    if (rotationRule == CLASSIC) {
        if (activePiece->pieceIndex != 0) {
            for (int i = 0; i < 2; i++) {
                activePiece->position.x += classicKicks[i][0];
                activePiece->position.y += classicKicks[i][1];

                if (isValidRotation(pieces[activePiece->pieceIndex][rotation], *activePiece, playField)) {
                    activePiece->rotIndex = rotation;
                    break;
                }
                else {
                    activePiece->position.x -= classicKicks[i][0];
                    activePiece->position.y -= classicKicks[i][1];
                }

            }
        }

        /* I kicks (if enabled) */
        if (rule.extraKicks && activePiece->pieceIndex == 0) {
            for (int i = 0; i < 5; i++) {
                if (i > 2 && (!isTouchingFloor(activePiece, playField) || (floorKicked))) {
                    continue;
                }
                else if (i <= 2 && !isTouchingStack(activePiece, playField)) {
                    continue;
                }
                activePiece->position.x += classicIKicks[i][0];
                activePiece->position.y -= classicIKicks[i][1];

                if (isValidRotation(pieces[activePiece->pieceIndex][rotation], *activePiece, playField)) {
                    activePiece->rotIndex = rotation;

                    if (floorKicked)
                        rotatedAfterKick = true;

                    if (i > 2)
                        floorKicked = true;

                    break;
                }
                else {
                    activePiece->position.x -= classicIKicks[i][0];
                    activePiece->position.y += classicIKicks[i][1];
                }

            }
        }
    }

    // World rule kicks (srs)
    if (rotationRule == WORLD) {
        for (int i = 0; i < 5; i++) {

            int kickIdx;
            int x;
            int y;
            // I kicks
            if (activePiece->pieceIndex == 0) {
                if (currentRot == 0 && rotationDir == 0) kickIdx = 7; // 0->L
                if (currentRot == 0 && rotationDir == 1) kickIdx = 0; // 0->R

                if (currentRot == 1 && rotationDir == 0) kickIdx = 1; // R->0
                if (currentRot == 1 && rotationDir == 1) kickIdx = 2; // R->2

                if (currentRot == 2 && rotationDir == 0) kickIdx = 3; // 2->R
                if (currentRot == 2 && rotationDir == 1) kickIdx = 4; // 2->L

                if (currentRot == 3 && rotationDir == 0) kickIdx = 5; // L->2
                if (currentRot == 3 && rotationDir == 1) kickIdx = 6; // L->0
                x = worldIKicks[kickIdx][i][0];
                y = worldIKicks[kickIdx][i][1];
            }
            else {
                if (currentRot == 0 && rotationDir == 0) kickIdx = 2; // 0->L
                if (currentRot == 0 && rotationDir == 1) kickIdx = 0; // 0->R

                if (currentRot == 1 && rotationDir == 0) kickIdx = 1; // R->0
                if (currentRot == 1 && rotationDir == 1) kickIdx = 1; // R->2

                if (currentRot == 2 && rotationDir == 0) kickIdx = 0; // 2->R
                if (currentRot == 2 && rotationDir == 1) kickIdx = 2; // 2->L

                if (currentRot == 3 && rotationDir == 0) kickIdx = 3; // L->2
                if (currentRot == 3 && rotationDir == 1) kickIdx = 3; // L->0

                x = worldKicks[kickIdx][i][0];
                y = worldKicks[kickIdx][i][1];
            }

            activePiece->position.x += x;
            activePiece->position.y -= y;

            if (isValidRotation(pieces[activePiece->pieceIndex][rotation], *activePiece, playField)) {
                activePiece->rotIndex = rotation;
                break;
            }
            else {
                activePiece->position.x -= x;
                activePiece->position.y += y;
            }
        }
    }
}


Vector2 getFinalPos(Piece* activePiece, PlayField playField)
{
    Vector2 finalPos = {0,0};
    Vector2 previousPos = {activePiece->position.x, activePiece->position.y};

    for (unsigned int yPos = activePiece->position.y - 1; yPos < playField.height; yPos++) {
        for (unsigned int y = 0; y < 4; y++) {
            for (unsigned int x = 0; x < 4; x++) {
                Vector2 currentPos = {activePiece->position.x, yPos};
                if (canMove(currentPos, activePiece, playField, 0, 1)) {
                    previousPos = currentPos;
                }
                else {
                    finalPos = previousPos;
                    finalPos.y ++;
                    return finalPos;
                }
            }
        }
    }

    return finalPos;
}


void checkIfAtBottom(Piece* activePiece, PlayField playField, SpeedSettings timings)
{
    if (TimerDone(appearanceDelayTimer) &&
        !canMove(activePiece->position, activePiece, playField, 0, 1))
    {
        lockIfFloorKicked(activePiece, playField, timings);
        if (GetElapsed(pieceLockDelayTimer) == 0) {
            PlaySound(landSound);
            startTimer(&pieceLockDelayTimer, timings.lockDelay);
        }
        else if (TimerDone(pieceLockDelayTimer)) {
            resetTimer(&pieceLockDelayTimer);
            startTimer(&appearanceDelayTimer, timings.appearanceDelay);
            activePiece->locked = true;
            startTimer(&flashTimer, framesToMilliseconds(3));
        }

    }
    else if (TimerDone(appearanceDelayTimer) &&
             canMove(activePiece->position, activePiece, playField, 0, 1))
    {
        resetTimer(&pieceLockDelayTimer);
    }

    if (TimerDone(flashTimer) && GetElapsed(flashTimer) != 0) {
        currentPieceToMatrix(activePiece, playField, timings);
        resetTimer(&flashTimer);
        activePiece->locked = false;
    }
}


void lockIfFloorKicked(Piece* activePiece, PlayField playField, SpeedSettings timings)
{
    if (!floorKicked ||
        !rotatedAfterKick ||
        canMove(activePiece->position, activePiece, playField, 0, 1))
        return;

    activePiece->locked = true;
    resetTimer(&pieceLockDelayTimer);
    startTimer(&appearanceDelayTimer, timings.appearanceDelay);
    startTimer(&flashTimer, framesToMilliseconds(3));
}


void moveDown(Piece* activePiece, PlayField playField, SpeedSettings timings)
{
    if (framesToMilliseconds(1) / timings.tickSpeed <= 1 &&
        TimerDone(appearanceDelayTimer) &&
        TimerDone(tickTimer))
    {
        startTimer(&tickTimer, timings.tickSpeed);

        if (canMove(activePiece->position, activePiece, playField, 0, 1)) {
            activePiece->position.y += 1;
        }
        else {
            checkIfAtBottom(activePiece, playField, timings);
        }
    }
    /* if gravity speed is faster than fps */
    else if (TimerDone(appearanceDelayTimer) && TimerDone(tickTimer) && !activePiece->locked) {
        int rows = framesToMilliseconds(1) / timings.tickSpeed;
        while (rows > 0) {
            if (canMove(activePiece->position, activePiece, playField, 0, 1)) {
                activePiece->position.y += 1;
            }
            else {
                checkIfAtBottom(activePiece, playField, timings);
                break;
            }
            rows --;
        }
    }
}


void softDrop(Piece* activePiece, PlayField playField, SpeedSettings* timings)
{
    if (!TimerDone(keyRepeatRateTimer)) {
        return;
    }

    if (canMove(activePiece->position, activePiece, playField, 0, 1)) {
        timings->linesSoftDropped += 1;
        activePiece->position.y += 1;
        startTimer(&keyRepeatRateTimer, timings->autoRepeatRate);
    }
    else if (rotationRule == CLASSIC) {
        pieceLockDelayTimer.startTime = timings->lockDelay;
    }
}


void movePiece(Piece* activePiece, PlayField playField, int dir, SpeedSettings timings)
{
    if (TimerDone(keyRepeatRateTimer) &&
        TimerDone(keyRepeatDelayTimer) &&
        canMove(activePiece->position, activePiece, playField, dir, 0))
    {
        activePiece->position.x += dir;
        startTimer(&keyRepeatRateTimer, timings.autoRepeatRate);
        PlaySound(moveSound);
        if (!canMove(activePiece->position, activePiece, playField, 0, 1)) {
            resetLockDelay(timings);
        }
    }
}


void moveIgnoreDAS(Piece* activePiece, PlayField playField, int dir, SpeedSettings timings)
{
    if (canMove(activePiece->position, activePiece, playField, dir, 0)) {
        activePiece->position.x += dir;
        PlaySound(moveSound);
        if (!canMove(activePiece->position, activePiece, playField, 0, 1)) {
            resetLockDelay(timings);
        }
    }
}


void rotateRight(Piece* activePiece, SpeedSettings rule, PlayField playField)
{
    if (!canRotate || !centerColumnCheck(activePiece, playField)) {
        return;
    }

    int tempRot = activePiece->rotIndex + 1;

    if (tempRot > 3) {
        tempRot = 0;
    }

    if (isValidRotation(pieces[activePiece->pieceIndex][tempRot], *activePiece, playField)) {
        activePiece->rotIndex = tempRot;
        if (floorKicked)
            rotatedAfterKick = true;
    }
    else {
        rotationDir = 1;
        performWallKick(activePiece, rule, playField, tempRot);
    }

    if (rotationRule == WORLD)
        resetLockDelay(rule);
}


void rotateLeft(Piece* activePiece, SpeedSettings rule, PlayField playField)
{
    if (!canRotate || !centerColumnCheck(activePiece, playField)) {
        return;
    }

    int tempRot = activePiece->rotIndex - 1;

    if (tempRot < 0) {
        tempRot = 3;
    }

    if (isValidRotation(pieces[activePiece->pieceIndex][tempRot], *activePiece, playField)) {
        activePiece->rotIndex = tempRot;
        if (floorKicked)
            rotatedAfterKick = true;
    }
    else {
        rotationDir = 0;
        performWallKick(activePiece, rule, playField, tempRot);
    }

    if (rotationRule == WORLD)
        resetLockDelay(rule);
}


PlayField initPlayField(int width, int height, Texture blockTileset)
{
    Block** matrix = calloc(height, sizeof(Block*));

    for (int y = 0; y < height; y++) {
        matrix[y] = calloc(width, sizeof(Block));
        for (int x = 0; x < width; x++) {
            matrix[y][x].type = 0;
            matrix[y][x].visible = true;
            matrix[y][x].tileset = blockTileset;
        }
    }

    PlayField playField;
    playField.matrix = matrix;
    playField.width = width;
    playField.height = height;

    return playField;
}


void unloadPlayField(PlayField playField)
{
    for (unsigned int y = 0; y < playField.height; y++) {
        free(playField.matrix[y]);
    }
    free(playField.matrix);
}


void processInput(Piece* activePiece, PlayField playField, SpeedSettings* timings)
{
    if (activePiece->locked) {
        return;
    }

    if (IsKeyPressed(config.keyBinds.ccw) || IsKeyPressed(config.keyBinds.ccwAlt))
        rotateLeft(activePiece, *timings, playField);

    if (IsKeyPressed(config.keyBinds.cw))
        rotateRight(activePiece, *timings, playField);

    if (IsKeyPressed(config.keyBinds.swapHold))
        holdSwap(activePiece, *timings, playField);

    if (IsKeyPressed(config.keyBinds.right)) {
        resetTimer(&keyRepeatDelayTimer);
        moveIgnoreDAS(activePiece, playField, 1, *timings);
        startTimer(&keyRepeatDelayTimer, timings->delayedAutoShift);
    }
    else if (IsKeyPressed(config.keyBinds.left)) {
        resetTimer(&keyRepeatDelayTimer);
        moveIgnoreDAS(activePiece, playField, -1, *timings);
        startTimer(&keyRepeatDelayTimer, timings->delayedAutoShift);
    }
    else if (IsKeyPressed(config.keyBinds.down)) {
        canSoftDrop = true;
        softDrop(activePiece, playField, timings);
    }
    else if (IsKeyPressed(config.keyBinds.sonicDrop) && timings->sonicDropEnabled) {
        activePiece->position = getFinalPos(activePiece, playField);

        // hard drop
        if (rotationRule == WORLD)
            pieceLockDelayTimer.startTime = timings->lockDelay;
    }

    if (IsKeyDown(config.keyBinds.right))
        movePiece(activePiece, playField, 1, *timings);

    else if (IsKeyDown(config.keyBinds.left))
        movePiece(activePiece, playField, -1, *timings);

    else if (IsKeyDown(config.keyBinds.down) && canSoftDrop)
        softDrop(activePiece, playField, timings);
}


bool processMenuInput(unsigned int* menuIndex, unsigned int max)
{
    bool gameEntered = false;

    if (IsKeyPressed(config.keyBinds.up)) {
        PlaySound(selectSound);
        if (*menuIndex == 0) {
            *menuIndex = max - 1;
        }
        else {
            *menuIndex -= 1;
        }
    }

    if (IsKeyPressed(config.keyBinds.down)) {
        PlaySound(selectSound);
        if (*menuIndex == max - 1) {
            *menuIndex = 0;
        }
        else {
            *menuIndex += 1;
        }
    }

    if (IsKeyPressed(config.keyBinds.uiSelect)) {
        gameEntered = true;
        PlaySound(lineClearSound);
    }

    return gameEntered;
}

