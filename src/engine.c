#include "engine.h"
#include "main.h"
#include "piece.h" // for pieces[7][4][4][4] global matrix containing every piece/rotation
#include <stdlib.h> // required for malloc(), free()
#include <stdio.h>
#include <time.h> // required for seeding RNG

Timer keyRepeatRateTimer;
Timer keyRepeatDelayTimer;
Timer tickTimer;
Timer pieceLockDelayTimer;
Timer lineClearDelayTimer;
Timer appearanceDelayTimer;

int upcomingPieces[3];
int recentPieces[4] = {4,4,5,5};
int heldPiece = -1;
int queuedPiece = -1;
int lockDelayResetCounter = 0;

bool canSoftDrop = true;
bool canHold = true;
bool canRotate = true;


bool canDrawPiece() {
    return (TimerDone(appearanceDelayTimer) && TimerDone(lineClearDelayTimer));
}


void shiftBag(int newPiece) {
    int newBag[4];
    for (int i = 0; i < 3; i++) {
        newBag[i+1] = recentPieces[i];
    }
    newBag[0] = newPiece;

    for (int i = 0; i < 4; i++) {
        recentPieces[i] = newBag[i];
    }
}

bool bagContains(int piece) {
    bool hasPiece = false;
    for (int i = 0; i < 4; i++) {
        if (recentPieces[i] == piece) {
            hasPiece = true;   
        }
    }
    return hasPiece;
}


bool isValidRotation(int piece[4][4], Piece* activePiece, Block** playField) {
    bool valid = false;
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (piece[y][x] != 0) {
                int xPos = activePiece->position.x;
                int yPos = activePiece->position.y;
                if (xPos+x > matrixWidth - 1 || xPos+x < 0 || yPos+y > matrixHeight - 1 || yPos+y < 0) {
                    valid = false;
                    goto exitRotationLoop;
                }
                if (playField[y+yPos][x+xPos].type == 0) {
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



int getRandomPiece(void) {
    int attempts = 0;
    int randomPiece;
    while (attempts != 6) {
        randomPiece = (rand() % (6 - 0 + 1)) + 0;
        
        if (!bagContains(randomPiece)) {
            shiftBag(randomPiece);
            break;
        }

        attempts ++;
    }

    return randomPiece;
}

void updatePreview(void) {
    int newArr[4];
    for (int i = 3; i >= 0; i--) {
        newArr[i-1] = upcomingPieces[i];
    }

    for (int i = 0; i < 4; i++) {
        upcomingPieces[i] = newArr[i];
    }

   upcomingPieces[2] = getRandomPiece(); 
}

void checkIfGameOver(Piece* activePiece, Block** playField) {
    bool gameOver = false;
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (pieces[activePiece->pieceIndex][activePiece->rotIndex][y][x] != 0) {
                int xPos = activePiece->position.x + x;
                int yPos = activePiece->position.y + y;

                if (playField[yPos][xPos].type != 0) {
                    gameOver = true;
                }
            }
        }
    }

    if (gameOver == true) {
        declareGameOver();
    }
}

void queuePiece(Piece* activePiece) {
    activePiece->position.x = 3.0f;
    activePiece->position.y = 1.0f;
    queuedPiece = upcomingPieces[0];
}

void spawnQueuedPiece(Piece* activePiece, float tickSpeed, Block** playField) {
    if (queuedPiece != -1 && TimerDone(appearanceDelayTimer) && TimerDone(lineClearDelayTimer)) {
        activePiece->rotIndex = 0;
        
        if (IsKeyDown(KEY_Z)) {
            activePiece->rotIndex = 3;
            PlaySound(preRotateSound);
        }
        else if (IsKeyDown(KEY_X)) {
            activePiece->rotIndex = 1;
            PlaySound(preRotateSound);
        }

        startTimer(&tickTimer, tickSpeed);
        canHold = true;
        canRotate = true;
        activePiece->pieceIndex = queuedPiece;
        queuedPiece = -1;
        updatePreview();

        activePiece->position.x = 3.0f;
        activePiece->position.y = 1.0f;
        
        checkIfGameOver(activePiece, playField);
        
        resetTimer(&appearanceDelayTimer);
        resetTimer(&lineClearDelayTimer);
    }
}

void holdSwap(Piece* activePiece) {
    if (canHold) {
        int currentPiece = activePiece->pieceIndex;
        if (heldPiece == -1) {
            activePiece->pieceIndex = upcomingPieces[0];
            updatePreview();
        } 
        else {
            activePiece->pieceIndex = heldPiece;
        }
        canHold = false;
        heldPiece = currentPiece;
        activePiece->position.x = 3.0f;
        activePiece->position.y = 1.0f;
        activePiece->rotIndex = 0;
    }
}

void generateInitialPreview(Piece* activePiece) {
    int attempts = 0;
    int randomPiece;
    
    for (int i = 0; i < 3; i++) {
        randomPiece = getRandomPiece();
        upcomingPieces[i] = randomPiece;
    }

    queuePiece(activePiece);
}

void fillRow(int row, int typeID, Block** playField) {
    for (int x = 0; x < matrixWidth; x++) {
        playField[row][x].type = typeID;
    }
}

void moveRowsDown(int blankRow, Block** playField) {
    for (int y = blankRow; y >= 0; y--) {
        for (int x = 0; x < matrixWidth; x++) {
            if (y - 1 != -1) {
                playField[y][x] = playField[y - 1][x];
            }
        }
    }
}

void checkForLineClear(Block** playField, float lineClearSpeed) {
    int lineCount = 0;
    for (int y = 0; y < matrixHeight; y++) {
        bool isFull = true;
        for (int x = 0; x < matrixWidth; x++) {
            if (playField[y][x].type == 0) {
                isFull = false;
            }
        }
        if (isFull) {
            canRotate = false; 
            fillRow(y, 0, playField);
            startTimer(&lineClearDelayTimer, lineClearSpeed);
            lineCount++;
            moveRowsDown(y, playField);
        }
    }

    if (lineCount == 4) {
        PlaySound(cheerSound);
    }
    else if (lineCount >= 1) {
        PlaySound(lineClearSound);
    }
    
    advanceLevel(lineCount);

}

void currentPieceToMatrix(Piece* activePiece, Block** playField, float lineClearSpeed) {
    canHold = false;
    canRotate = false;
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (pieces[activePiece->pieceIndex][activePiece->rotIndex][y][x] != 0) {
                int xPos = activePiece->position.x;
                int yPos = activePiece->position.y;
                playField[y+yPos][x+xPos].type = pieces[activePiece->pieceIndex][activePiece->rotIndex][y][x];
            }
        }
    }
    
    PlaySound(pieceLockSound);
    checkForLineClear(playField, lineClearSpeed);
    queuePiece(activePiece);
}

void resetLockDelay(float lockDelay) {
    if (lockDelayResetCounter < 3) {
        resetTimer(&pieceLockDelayTimer);
        startTimer(&pieceLockDelayTimer, lockDelay);
        lockDelayResetCounter ++;
    }
}


bool canMove(Vector2 position, Piece* activePiece, Block** playField, int dirX, int dirY) {
    bool move = false;

    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (pieces[activePiece->pieceIndex][activePiece->rotIndex][y][x]) {
                int xPos = x + position.x + dirX;
                int yPos = y + position.y + dirY;

                if (xPos <= matrixWidth - 1 && xPos >= 0 && yPos <= matrixHeight - 1 && yPos >= 0) {
                    int dirBlock = playField[yPos][xPos].type;
                    if (dirBlock == 0) {
                        move = true;
                    } 
                    else if (dirBlock != 0 && pieces[activePiece->pieceIndex][activePiece->rotIndex][y][x] != 0){
                        move = false;
                        goto exitLoop;
                    }
                }
                else {
                    move = false;
                    goto exitLoop;
                }
            }

        }
    }
    // https://xkcd.com/292/
    exitLoop:
        return move;
}

bool isTouchingStack(Piece* activePiece, Block** playField) {
    bool touchingStack = false;

    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            int xPos = activePiece->position.x + x;
            int yPos = activePiece->position.y + y;
            if (pieces[activePiece->pieceIndex][activePiece->rotIndex][y][x] != 0) {
                if (xPos - 1 < 0) {
                    touchingStack = true; 
                    goto exitTouchingStackLoop;
                }
                else if (playField[yPos][xPos - 1].type != 0) {
                    touchingStack = true; 
                    goto exitTouchingStackLoop;
                }

                if (xPos + 1 > 19) {
                    touchingStack = true; 
                    goto exitTouchingStackLoop;
                } 
                else if (playField[yPos][xPos + 1].type != 0) {
                    touchingStack = true; 
                    goto exitTouchingStackLoop; 
                }
            }
        }
    }
    
    exitTouchingStackLoop:
        return touchingStack;
}

void performFloorKick(Piece* activePiece, Block** playField, int rotation) {
    // I piece floor kick
    if (activePiece->pieceIndex == 0) {
        activePiece->position.y -= 1; // kick up 1

        if (!isValidRotation(pieces[activePiece->pieceIndex][rotation], activePiece, playField)) {
            activePiece->position.y -= 1; // kick up 2
            
            if (!isValidRotation(pieces[activePiece->pieceIndex][rotation], activePiece, playField)) {
                activePiece->position.y += 2; // return to original position
            }
        }

        if (isValidRotation(pieces[activePiece->pieceIndex][rotation], activePiece, playField)) {
            activePiece->rotIndex = rotation;
        }
    }

    // T piece floor kick
    if (activePiece->pieceIndex == 1) {
        activePiece->position.y -= 1; // kick up 1
        if (!isValidRotation(pieces[activePiece->pieceIndex][rotation], activePiece, playField)) {
            activePiece->position.y += 1; // return to original position
        }
        else {
            activePiece->rotIndex = rotation;
        }
    }
}

void performWallKick(Piece* activePiece, Block** playField, int rotation) {
    bool isOnFloor = !canMove(activePiece->position, activePiece, playField, 0, 1);
    
    if (!isOnFloor && isTouchingStack(activePiece, playField)) {
        activePiece->position.x += 1;

        if (!isValidRotation(pieces[activePiece->pieceIndex][rotation], activePiece, playField)) {
            activePiece->position.x -= 2; // move left one

            if (!isValidRotation(pieces[activePiece->pieceIndex][rotation], activePiece, playField)) {
                activePiece->position.x += 1; // move back to original position

                if (!isValidRotation(pieces[activePiece->pieceIndex][rotation], activePiece, playField)) {
                    activePiece->position.x += 2; // kick I piece to the right 2 spaces

                    if (!isValidRotation(pieces[activePiece->pieceIndex][rotation], activePiece, playField)) {
                        activePiece->position.x -= 2; // kick I piece back to original position
                    }
                }
            }
        }

        if (isValidRotation(pieces[activePiece->pieceIndex][rotation], activePiece, playField)) {
            activePiece->rotIndex = rotation;
        }
    }
    else if(isOnFloor) {
        performFloorKick(activePiece, playField, rotation);
    }


}

Vector2 getFinalPos(Piece* activePiece, Block** playField) {
    Vector2 finalPos = {0,0};
    Vector2 previousPos = {activePiece->position.x, activePiece->position.y};

    for (int yPos = activePiece->position.y - 1; yPos < matrixHeight; yPos++) {
        for (int y = 0; y < 4; y++) {
            for (int x = 0; x < 4; x++) {
                Vector2 currentPos = {activePiece->position.x, yPos};
                if (canMove(currentPos, activePiece, playField, 0, 1)) {
                    previousPos = currentPos;
                }
                else {
                    finalPos = previousPos;
                    finalPos.y ++;
                    goto exitFinalPosLoop;
                }
            }
        }
    }
    exitFinalPosLoop:
        return finalPos;
}

void checkIfAtBottom(Piece* activePiece, Block** playField, float lineClearSpeed, float lockDelay, float appearanceDelay) {
    if (TimerDone(appearanceDelayTimer) && !canMove(activePiece->position, activePiece, playField, 0, 1)) {

        if (GetElapsed(pieceLockDelayTimer) == 0) {
            PlaySound(landSound);
            startTimer(&pieceLockDelayTimer, lockDelay);
        }
        else if (TimerDone(pieceLockDelayTimer)) { 
            resetTimer(&pieceLockDelayTimer);
            lockDelayResetCounter = 0;
            startTimer(&appearanceDelayTimer, appearanceDelay);
            currentPieceToMatrix(activePiece, playField, lineClearSpeed);
        }

    } 
    else {
        resetTimer(&pieceLockDelayTimer);
    }
}

void moveDown(Piece* activePiece, Block** playField, float tickSpeed, float lineClearSpeed, float lockDelay, float appearanceDelay, bool gravity20G) {
    if (!gravity20G) {
        if (TimerDone(appearanceDelayTimer)) {
            if (GetElapsed(tickTimer) == 0) {
                startTimer(&tickTimer, tickSpeed);
            } 
            else if (TimerDone(tickTimer)){
                startTimer(&tickTimer, tickSpeed);
                
                if (canMove(activePiece->position, activePiece, playField, 0, 1)) {
                    activePiece->position.y += 1;
                } 
                else {
                    checkIfAtBottom(activePiece, playField, lineClearSpeed, lockDelay, appearanceDelay);
                }
            }
        }
    } 
    else if (TimerDone(appearanceDelayTimer)) {
        if (canMove(activePiece->position, activePiece, playField, 0, 1)) {
            activePiece->position = getFinalPos(activePiece, playField);
        }
        else {
            checkIfAtBottom(activePiece, playField, lineClearSpeed, lockDelay, appearanceDelay);
        }
    }
}


void softDrop(Piece* activePiece, Block** playField, float lockDelay, float autoRepeatRate) {
    if (TimerDone(keyRepeatRateTimer) && canSoftDrop) {
        if (canMove(activePiece->position, activePiece, playField, 0, 1)) {
            activePiece->position.y += 1;
            startTimer(&keyRepeatRateTimer, autoRepeatRate);
            PlaySound(moveSound);
        } 
        else {
            pieceLockDelayTimer.startTime = lockDelay;
            canSoftDrop = false;
        }
    }
}

void movePiece(Piece* activePiece, Block** playField, int dir, float lockDelay, float autoRepeatRate) {
    if (TimerDone(keyRepeatRateTimer) && TimerDone(keyRepeatDelayTimer) && canMove(activePiece->position, activePiece, playField, dir, 0)) {
        activePiece->position.x += dir;
        startTimer(&keyRepeatRateTimer, autoRepeatRate);
        PlaySound(moveSound);
    }
    else if (!canMove(activePiece->position, activePiece, playField, 0, 1)) {
        resetLockDelay(lockDelay);
    }
}

void moveResetDAS(Piece* activePiece, Block** playField, int dir, float lockDelay) {
    if (canMove(activePiece->position, activePiece, playField, dir, 0)) {
        activePiece->position.x += dir;
        PlaySound(moveSound);
    }
    else {
        resetLockDelay(lockDelay);
    }
}

void rotateRight(Piece* activePiece, Block** playField) {
    if (canRotate) {
        int tempRot = activePiece->rotIndex + 1;
        if (tempRot > 3) {
            tempRot = 0;
        } 
        if (isValidRotation(pieces[activePiece->pieceIndex][tempRot], activePiece, playField)) {
            activePiece->rotIndex = tempRot;
        }
        else {
            performWallKick(activePiece, playField, tempRot);
        }
    }
}

void rotateLeft(Piece* activePiece, Block** playField) {
    if (canRotate) {
        int tempRot = activePiece->rotIndex - 1;
        if (tempRot < 0) {
            tempRot = 3;
        }
        if (isValidRotation(pieces[activePiece->pieceIndex][tempRot], activePiece, playField)) {
            activePiece->rotIndex = tempRot;
        }
        else {
            performWallKick(activePiece, playField, tempRot);
        }
    }
}

Block** initMatrix(int width, int height, Texture blockTileset) {
    Block** matrix = calloc(height, sizeof(Block*));

    for (int y = 0; y < height; y++) {
        matrix[y] = calloc(width, sizeof(Block));
        for (int x = 0; x < width; x++) {
            matrix[y][x].type = 0;
            matrix[y][x].visible = true;
            matrix[y][x].tileset = blockTileset;
        }   
    }   

    return matrix;
}

void unloadMatrix(Block** matrix, int height) {
    for (int y = 0; y < height; y++) {
        free(matrix[y]);
    }
    free(matrix);
}



void processInput(Piece* activePiece, Block** playField, float delayedAutoShift, float autoRepeatRate, float lockDelay) {

    if (IsKeyPressed(KEY_Z)) {
        rotateLeft(activePiece, playField);
    } 
    
    if (IsKeyPressed(KEY_X)) {
        rotateRight(activePiece, playField);
    }

    if (IsKeyPressed(KEY_SPACE)) {
        holdSwap(activePiece);
    }

    if (IsKeyPressed(KEY_RIGHT)) {
        resetTimer(&keyRepeatDelayTimer);
        moveResetDAS(activePiece, playField, 1, lockDelay);
        startTimer(&keyRepeatDelayTimer, delayedAutoShift);
    } 
    else if (IsKeyPressed(KEY_LEFT)) {
        resetTimer(&keyRepeatDelayTimer);
        moveResetDAS(activePiece, playField, -1, lockDelay);
        startTimer(&keyRepeatDelayTimer, delayedAutoShift);
    } 
    else if (IsKeyPressed(KEY_DOWN)) {
        softDrop(activePiece, playField, lockDelay, autoRepeatRate);
    }

    if (IsKeyDown(KEY_RIGHT)) {
        movePiece(activePiece, playField, 1, lockDelay, autoRepeatRate);
    } else if (IsKeyDown(KEY_LEFT)) {
        movePiece(activePiece, playField, -1, lockDelay, autoRepeatRate);
    } else if (IsKeyDown(KEY_DOWN)) {
        softDrop(activePiece, playField, lockDelay, autoRepeatRate);
    }

    if (IsKeyReleased(KEY_DOWN)){
        canSoftDrop = true;
    }

    if (IsKeyPressed(KEY_UP)) {
        activePiece->position = getFinalPos(activePiece, playField);
    }

}

bool processMenuInput(int* gameType, Timer* countDown, int seconds) {
    bool gameEntered = false;
    
    if (IsKeyPressed(KEY_UP)) {
        PlaySound(selectSound);
        if (*gameType == 0) {
            *gameType = 4;
        }
        else {
            *gameType -= 1;
        }
    }

    if (IsKeyPressed(KEY_DOWN)) {
        PlaySound(selectSound);
        if (*gameType == 4) {
            *gameType = 0;
        }
        else {
            *gameType += 1;
        }
    }

    if (IsKeyPressed(KEY_ENTER)) {
        startTimer(countDown, seconds);
        gameEntered = true;
    }

    return gameEntered;
}

bool processPauseMenuInput(int* idxOption) {
    bool isSelected = false;

    if (IsKeyPressed(KEY_UP)) {
        PlaySound(selectSound);
        if (*idxOption <= 0) {
            *idxOption = 2;
        } else {
            *idxOption -= 1;
        }
    }

    if (IsKeyPressed(KEY_DOWN)) {
        PlaySound(selectSound);
        if (*idxOption >= 2) {
            *idxOption = 0;
        } else {
            *idxOption += 1;
        }
    }

    if (IsKeyPressed(KEY_ENTER)) {
        isSelected = true;
    }

    return isSelected;
}
