#include "main.h"
#include "draw.h"   // Functions that draw the pieces, playing field, etc to the screen
//#include "engine.h"
#include <time.h>   // required for seeding RNG
#include <stdlib.h> // required for srand
#include <stdio.h> // required for srand


static const int screenWidth = 1024;
static const int screenHeight = 704;

const int cellSize = 32;

const int matrixHeight = 22;
const int matrixWidth = 10;
Block** playField; // 2d array for playing area
Vector2 playFieldPos = {160, -32};

Texture2D blockTileset;
Texture2D frameTileset;
Texture2D background;

// master mode backgrounds
Texture2D bg000;
Texture2D bg100;
Texture2D bg200;
Texture2D bg300;
Texture2D bg400;
Texture2D bg500;

// death mode background
Texture2D deathBG000;

// easy mode background
Texture2D easy000;

// invisible mode background
Texture2D invisBG;

Color frameColor; // border frame color

Sound pieceLockSound;
Sound landSound;
Sound cheerSound;
Sound lineClearSound;
Sound moveSound;
Sound preRotateSound;

Piece activePiece;

int framesCounter; // used for pause blinking animation
int gameType = 0;
int currentLevel = 0;
int maxLevel = 0;
char debugText[20];
int idxPauseOption = 0;

bool gravity20G = false;
bool gameOver = false;
bool invisiblePieces = false;
bool inMenu = true;
bool pause = false;
bool shouldQuit = false;

float delayedAutoShift = 0.13f; // When you hold down a key this is the amount of time it takes for key repeat to activate
float autoRepeatRate = 20.0f; // The time between automatic keypresses while holding down a key
float appearanceDelay = 0.45f; // Appearance delay of spawned pieces
float lockDelay = 0.5f; // Time the piece takes to lock to the grid (in seconds)
float lineClearSpeed = 0.67f;
double tickSpeed = 1.f;

// I = 0
// T = 1
// L = 2
// J = 3
// S = 4
// Z = 5
// O = 6

// function prototypes
static void start();       // Game variables are initialized here
static void update(void);  // Game Logic
static void render(void);  // Drawing/Rendering the game
static void cleanUp(void); // Cleanup, free allocated memory, close window, etc
void updateLevel(void);    // change speed timings based on current game mode and level
double framesToMilliseconds(int frames); // Convert frames to miliseconds

int main(void) {
    InitWindow(screenWidth, screenHeight, "tuxmino v0.1");
    SetTargetFPS(60); // lock game to 60 frames per second
    SetExitKey(KEY_NULL);
    InitAudioDevice();

    start();

    // main game loop
    while (!WindowShouldClose() && !shouldQuit) {
        update();
        render();
    }
    cleanUp();

    return 0;
}

void start(void) {
    blockTileset = LoadTexture("res/block-tilesets/new-blocks.png");
    frameTileset = LoadTexture("res/decor/frame.png");
    bg000 = LoadTexture("res/bg/ring.png");
    bg100 = LoadTexture("res/bg/jupiter.png");
    bg200 = LoadTexture("res/bg/cartwheel.png");
    bg300 = LoadTexture("res/bg/tarantula.png");
    bg400 = LoadTexture("res/bg/bg400.png");
    bg500 = LoadTexture("res/bg/bg500.png");
    
    // death mode backgrounds
    deathBG000 = LoadTexture("res/bg/deathmode000.png");

    // easy mode backgrounds
    easy000 = LoadTexture("res/bg/easy/easy000.png");

    // invisible mode backgrounds
    invisBG = LoadTexture("res/bg/invisiblemode.png");
    
    background = bg000;

    cheerSound = LoadSound("res/snd/cheer.wav");
    landSound = LoadSound("res/snd/sonicDrop.wav");
    pieceLockSound = LoadSound("res/snd/lock.wav");
    lineClearSound = LoadSound("res/snd/lineClear.wav");
    moveSound = LoadSound("res/snd/move.wav");
    preRotateSound = LoadSound("res/snd/preRotate.wav");

    activePiece.tileset = blockTileset;
    
    playField = initMatrix(matrixWidth, matrixHeight, blockTileset);

    srand(time(0));
    updateLevel();
    generateInitialPreview(&activePiece);
}

void update(void) {
    
    if (IsKeyPressed(KEY_ESCAPE) && !inMenu) {
        pause = !pause;
    }

    if (IsKeyPressed(KEY_R)) {
        resetGame();
    }

    if (IsKeyPressed(KEY_S)) {
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        TakeScreenshot(TextFormat("screenshots/%d-%02d-%02d_%02d-%02d-%02d.png", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec));
    }

    if (!gameOver && !inMenu && !pause) {
        spawnQueuedPiece(&activePiece, tickSpeed, playField);
        checkIfAtBottom(&activePiece, playField, lineClearSpeed, lockDelay, appearanceDelay);
        processInput(&activePiece, playField, delayedAutoShift, autoRepeatRate, lockDelay);
        moveDown(&activePiece, playField, tickSpeed, lineClearSpeed, lockDelay, appearanceDelay, gravity20G);
    }
    else if (inMenu){
        inMenu = !processMenuInput(&gameType);
        updateLevel();
    }
    else if (gameOver) {
        invisiblePieces = false;
    }
    else if (pause) {
        framesCounter++;

        bool isSelected = processPauseMenuInput(&idxPauseOption);
        if (isSelected) {
            if (idxPauseOption == 0) {
                pause = false;
            } else if (idxPauseOption == 1) {
                shouldQuit = true;
            }
        }
    }
}

void render(void) {
    BeginDrawing();
        ClearBackground(BLACK);
        DrawTexture(background, 0, 0, WHITE);
        
        drawPlayField(playField, playFieldPos, cellSize, invisiblePieces); 
        drawPiecePreview(activePiece, playFieldPos, cellSize);
        drawheldPiece(activePiece, cellSize);
        drawStackOutline(playField, playFieldPos, cellSize, invisiblePieces);

        if (!gameOver && !inMenu) {
            drawActivePiece(activePiece, playFieldPos, cellSize);
            drawGhostPiece(&activePiece, playField, playFieldPos, cellSize);
        }
        else if (inMenu) {
            drawMenu(gameType, playFieldPos);
        }
        else {
            drawActivePiece(activePiece, playFieldPos, cellSize);
            DrawText("GAME OVER", playFieldPos.x, 200, 52, WHITE);
        }

        if (pause) {
            if ((framesCounter/30)%2) {
                DrawText("Paused", playFieldPos.x + (matrixWidth * cellSize) + 300, playFieldPos.y + ((matrixHeight * 32)/2), 40, WHITE);
            }

            drawPauseMenu(idxPauseOption, playFieldPos);
        }
        
        drawBorder(playFieldPos, frameTileset, cellSize, frameColor);

        DrawText(TextFormat("%03d", currentLevel), playFieldPos.x + (matrixWidth * 32) + 55, 500, 40, WHITE);
        DrawText(TextFormat("%03d", maxLevel), playFieldPos.x + (matrixWidth * 32) + 55, 540, 40, WHITE);

        DrawText(debugText, 500, screenHeight-30, 25, WHITE); 
        DrawFPS(10, screenHeight-30);

    EndDrawing();
}

void cleanUp(void) {
    unloadMatrix(playField, matrixHeight);

    UnloadTexture(blockTileset);
    UnloadTexture(bg000);
    UnloadTexture(bg100);
    UnloadTexture(bg200);
    UnloadTexture(bg300);
    UnloadTexture(bg400);
    UnloadTexture(bg500);

    UnloadTexture(deathBG000);

    UnloadTexture(easy000);
    UnloadTexture(invisBG);

    UnloadSound(pieceLockSound);
    UnloadSound(landSound);
    UnloadSound(cheerSound);
    UnloadSound(lineClearSound);
    UnloadSound(moveSound);
    UnloadSound(preRotateSound);
    
    CloseWindow();
}

void setDefaultTimings() {
    delayedAutoShift = framesToMilliseconds(16);
    autoRepeatRate = 0.01f;
    lockDelay = framesToMilliseconds(30);
    lineClearSpeed = framesToMilliseconds(40);
    appearanceDelay = framesToMilliseconds(27);
    gravity20G = false;
    invisiblePieces = false;
}

void updateLevel(void) {
    // master game mode
    if (gameType == 0) {
        frameColor = BLUE;
        // speed level one
        if (currentLevel <= 99) {
            setDefaultTimings();
            tickSpeed = 1.f;
            background = bg000;
        }
        else if (currentLevel <= 199) {
            setDefaultTimings();
            tickSpeed = 0.1f;
            background = bg100;
        }
        else if (currentLevel <= 299) {
            setDefaultTimings();
            tickSpeed = 0.05f;
            background = bg200;
        }
        else if (currentLevel <= 399) {
            setDefaultTimings();
            tickSpeed = 0.0025f;
            background = bg300;
        }
        else if (currentLevel <= 499) {
            setDefaultTimings();
            tickSpeed = 0.0025f;
            lockDelay = framesToMilliseconds(25);
            background = bg400;
        }
        else if (currentLevel <= 599) {
            setDefaultTimings();
            gravity20G = true;
            background = bg500;
        }
    }
    
    // death game mode
    else if (gameType == 1) {
        frameColor = RED;
        delayedAutoShift = framesToMilliseconds(10);
        autoRepeatRate = 0.01f;
        lockDelay = framesToMilliseconds(18);
        lineClearSpeed = framesToMilliseconds(6);
        appearanceDelay = framesToMilliseconds(12);
        gravity20G = true; 
        invisiblePieces = false;
        background = deathBG000;
    }

    // easy game mode
    else if (gameType == 2) {
        frameColor = GREEN;
        background = easy000;
        if (currentLevel <= 99) {
            setDefaultTimings();
            tickSpeed = 1.f;
        }
    }
    
    // invisible game mode
    else if (gameType == 3) {
        frameColor = YELLOW;
        setDefaultTimings();
        invisiblePieces = true;
        tickSpeed = 1.f;
        background = invisBG;
    }
    
    // 20G practice
    else if (gameType == 4) {
        frameColor = PURPLE;
        setDefaultTimings();
        gravity20G = true;
        background = bg500;
    }
}


void advanceLevel(int lineCount) {

    // dont increment the level if it is 1 level away from the level goal
    // this is so that you can only enter the next set of levels if you score a line clear
    if (currentLevel != maxLevel - 1) {
        currentLevel ++;
    }

    // 4 line clears = 6 levels
    // 3 line clears = 4 levels
    // anything under that = the amount of lines you cleared (1 or 2)
    if (lineCount == 4) {
        currentLevel += 6;
    }
    else if (lineCount == 3) {
        currentLevel += 4;
    }
    else {
        currentLevel += lineCount;
    }
    
    if (currentLevel >= maxLevel && currentLevel - maxLevel >= 0) {
        maxLevel += 100;
    }
    updateLevel();
}

void declareGameOver() {
    gameOver = true;
}

void resetGame() {
    heldPiece = -1;
    inMenu = true;
    gameOver = false;
    maxLevel = 0;
    currentLevel = 0;
    generateInitialPreview(&activePiece);
    playField = initMatrix(matrixWidth, matrixHeight, blockTileset);
}

double framesToMilliseconds(int frames) {
    return frames * 0.0166670f;
}
