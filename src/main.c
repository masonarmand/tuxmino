/*
 * File: main.c
 * ------------
 *
 * Original Author: Mason Armand
 * Contributors: Dawnvoid
 * Date Created: Sep 14, 2022
 * Last Modified: Mar 21, 2023
 */

#include "tuxmino.h"
#include <time.h>   // required for screenshot names
#include <stdlib.h>

#define SCREEN_SCALE_COUNT 5
#define SCREEN_WIDTH 1224
#define SCREEN_HEIGHT 820
#define PLAYFIELD_YSTART 29
#define PAUSE_OPTIONS 3
#define TITLE_OPTIONS 3

/* Global Externs */
Config config;
Sound pieceLockSound;
Sound landSound;
Sound cheerSound;
Sound lineClearSound;
Sound moveSound;
Sound preRotateSound;
Sound selectSound;
Sound boardFall;
Sound readySound;
Sound goSound;
Sound pieceSounds[7]; // piece spawn sound effects


static enum gameScreens currentScreen;

static PlayField playField;

static RenderTexture2D screenTexture;

static Texture2D blockTileset;
static Texture2D monochromeTileset;
static Texture2D worldTileset;
static Texture2D classicTileset;
static Texture2D frameTileset;
static Texture2D gradeTileset;
static Texture2D altGradeTileset;
static Texture2D explosionEffectTileset;
static RenderTexture2D creditsTexture;
static Texture2D nextPieceBg;
static Texture2D titlescreenBG;
static Color fade; // piece fade color when locking to grid

static Piece activePiece;

static Timer delayStartTimer;
static Timer lineClearEffectFrameTime;
static Timer lockFadeTime;
//static GameTimer gameTimer;

static unsigned int modeId = 0;
static unsigned int idxPauseOption = 0;
static unsigned int idxTitleOption = 0;
static unsigned int idxGameModeSettingOption;

// Used by the countdown timer to check when to play countdown sound.
static int lastCount = -1;
static int startCountDown = 3;

// for line clear explosion animation
static int explosionCurrentFrame;
static int explosionCurrentLine;

static bool gameOver = false;
static bool inCreditRoll = false;
static bool inMenu = true;
static bool inGameModeSettings = false;
static bool pause = false;
static bool shouldQuit = false;
static bool lineClearEffect = false;
static bool lockFade = false;

static GameModeList gameModes;

// function prototypes
static void start(void);       // Game variables are initialized here
static void update(void);  // Game Logic
static void render(void);  // Drawing/Rendering the game
static void cleanUp(void); // Cleanup, free allocated memory, close window, etc
void updateLevel(void);

int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "tuxmino v1.0.0");
    SetExitKey(KEY_NULL);
    InitAudioDevice();

    if (ini_parse("config.ini", handler, &config) < 0) {
        printf("Can't load 'config.ini'\n");
        return 1;
    }
    printf("INI: Config loaded from 'config.ini'\n");
    SetWindowSize(config.settings.displayWidth, config.settings.displayHeight);
    SetTargetFPS(config.settings.fps);

    SetWindowIcon(LoadImage("res/ui/logo.png"));
    start();
    // main game loop
    while (!WindowShouldClose() && !shouldQuit) {
        //recordInputs();
        update();
        render();
    }

    cleanUp();

    return 0;
}

void start(void) {
    // -------------------------------------------------------------------------
    // Load Game Modes
    // -------------------------------------------------------------------------

    loadGameModes(&gameModes);

    // -------------------------------------------------------------------------
    // Initialze PlayField
    // -------------------------------------------------------------------------
    playField = initPlayField(10, 22, blockTileset);
    playField.copy = initPlayField(10, 22, blockTileset).matrix;

    playField.cellSize = 32;
    playField.pos = (Vector2) {
        (SCREEN_WIDTH/2) - ((playField.width * playField.cellSize) / 2), PLAYFIELD_YSTART
    };
    playField.blankBlockTexture = LoadTexture("res/block-tilesets/gray.png");

    // -------------------------------------------------------------------------
    // Texture Initialization
    // -------------------------------------------------------------------------

    screenTexture = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);

    monochromeTileset = LoadTexture("res/block-tilesets/monochrome-blocks.png");
    worldTileset = LoadTexture("res/block-tilesets/world-blocks.png");
    classicTileset = LoadTexture("res/block-tilesets/blocks.png");
    blockTileset = classicTileset;
    frameTileset = LoadTexture("res/decor/frame.png");
    gradeTileset = LoadTexture("res/decor/grades.png");
    altGradeTileset = LoadTexture("res/decor/grades1.png");
    explosionEffectTileset = LoadTexture("res/animations/break0.png");
    //creditsTexture.texture = LoadTexture("res/decor/credits.png");
    creditsTexture = renderCreditsTexture(playField);
    nextPieceBg = LoadTexture("res/decor/nextPieceBg.png");

    titlescreenBG = LoadTexture("res/bg/titlescreen.png");

    // -------------------------------------------------------------------------
    // Sound Initialization
    // -------------------------------------------------------------------------

    cheerSound = LoadSound("res/snd/cheer.wav");
    SetSoundVolume(cheerSound, 0.5f);

    landSound = LoadSound("res/snd/land.wav");
    SetSoundVolume(landSound, 0.4f);

    pieceLockSound = LoadSound("res/snd/lock.wav");
    SetSoundVolume(pieceLockSound, 0.4f);
    lineClearSound = LoadSound("res/snd/lineClear.wav");
    moveSound = LoadSound("res/snd/move.wav");

    preRotateSound = LoadSound("res/snd/preRotate.wav");
    SetSoundVolume(preRotateSound, 0.2f);

    selectSound = LoadSound("res/snd/select.wav");

    boardFall = LoadSound("res/snd/boardFall.wav");
    SetSoundVolume(boardFall, 0.5f);

    readySound = LoadSound("res/snd/ready.wav");
    goSound = LoadSound("res/snd/go.wav");

    for (int i = 0; i < 7; i++) pieceSounds[i] = LoadSound(TextFormat("res/snd/pieceSpawn/%i.wav", i));
    for (int i = 0; i < 7; i++) SetSoundVolume(pieceSounds[i], 0.4f);

    // -------------------------------------------------------------------------
    // Misc Initialization
    // -------------------------------------------------------------------------

    fade = WHITE;
    setRotationRule(0);
    activePiece.tileset = blockTileset;
    activePiece.currentTileset = blockTileset;
    activePiece.boneBlocks = monochromeTileset;
    activePiece.locked = false;
    currentScreen = TITLE;
    generateInitialPreview(&activePiece, playField, gameModes.modes[modeId].rule);
}

void update(void) {
    updateLevel();
    SpeedSettings rule = gameModes.modes[modeId].rule;

    if (gameModes.modes[modeId].boneBlocks) {
        activePiece.currentTileset = activePiece.boneBlocks;
    }
    else {
        activePiece.currentTileset = activePiece.tileset;
    }

    if (currentScreen == TITLE) {
        bool isSelected = processMenuInput(&idxTitleOption, TITLE_OPTIONS);
        if (isSelected) {
            switch (idxTitleOption) {
                case 0: currentScreen = GAME_MODES; break;
                case 1: currentScreen = OPTIONS; break;
                case 2: shouldQuit = true; break;
            }
        }
    }
    else if (currentScreen == OPTIONS) {
        if (IsKeyPressed(KEY_ESCAPE)) {
            currentScreen = TITLE;
        }
        //TODO
    }
    else if (currentScreen == GAME_MODES) {

        if (IsKeyPressed(KEY_ESCAPE) && !inMenu && !inGameModeSettings && !gameOver) {
            idxPauseOption = 0;
            pause = !pause;
            togglePauseGameTimer(&gameModes.modes[modeId].gameTimer);
        }
        else if (IsKeyPressed(KEY_ESCAPE) && inMenu) {
            currentScreen = TITLE;
        }

        if (IsKeyPressed(config.keyBinds.screenshot)) {
            time_t t = time(NULL);
            struct tm tm = *localtime(&t);
            TakeScreenshot(TextFormat("screenshots/%d-%02d-%02d_%02d-%02d-%02d.png", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec));
        }

        if (IsKeyPressed(config.keyBinds.restart)) {
            resetGame();
            inMenu = false;
            inGameModeSettings = true;
        }

        if (IsKeyReleased(config.keyBinds.uiSelect) && gameOver) {
            resetGame();
        }

        if (!gameOver && !inMenu && !inGameModeSettings && !pause && TimerDone(delayStartTimer)) {
            processInput(&activePiece, playField, &gameModes.modes[modeId].rule);
            moveQueuedLinesDown(playField);
            spawnQueuedPiece(&activePiece, playField, rule);
            checkIfAtBottom(&activePiece, playField, rule);
            moveDown(&activePiece, playField, rule);

            //if (GetElapsed(rule.creditRollTimer) != 0 && TimerDone(rule.creditRollTimer)) declareGameOver();

            if (!inCreditRoll && rule.creditRoll == true) {
                inCreditRoll = true;

                if (rule.clearOnCredits) {
                    for (size_t y = 0; y < playField.height; y++) {
                        for (size_t x = 0; x < playField.width; x++) {
                            playField.matrix[y][x].type = 0;
                            playField.copy[y][x].type = 0;
                        }
                    }
                }
            }

            //------------------------------------------------------------------
            // Explosion line clear effect animation is calculated here
            //------------------------------------------------------------------

            if (lineClearEffect) {
                if (lineClearEffectFrameTime.startTime == 0)
                    startTimer(&lineClearEffectFrameTime, rule.lineClearSpeed/35);

                if (TimerDone(lineClearEffectFrameTime)) {
                    startTimer(&lineClearEffectFrameTime, rule.lineClearSpeed/35);
                    explosionCurrentFrame++;
                    // if the last frame in the sprite sheet is reached, go to
                    // the next line in the spritesheet
                    if (explosionCurrentFrame >= 6) {
                        explosionCurrentFrame = 0;
                        explosionCurrentLine++;

                        if (explosionCurrentLine >= 6) {
                            explosionCurrentFrame = 0;
                            explosionCurrentLine = 0;
                            lineClearEffect = false;
                        }
                    }
                }

            }
            else {
                explosionCurrentFrame = 0;
                explosionCurrentLine = 0;
            }

            lockFade = !canMove(activePiece.position, &activePiece, playField, 0, 1);

            //------------------------------------------------------------------
            // Lock Delay Fading Animation Calculation is done here.
            //------------------------------------------------------------------
            if (lockFade) {
                if (lockFadeTime.startTime == 0)
                    startTimer(&lockFadeTime, framesToMilliseconds(1));

                if (TimerDone(lockFadeTime)) {
                    startTimer(&lockFadeTime, framesToMilliseconds(1));
                    fade.r -= 2;
                    fade.g -= 2;
                    fade.b -= 2;
                    if (fade.r <= 130) {
                        fade = (Color){255, 255, 255, 255};
                    }
                }
            }
            else {
                fade = WHITE;
                lockFadeTime.startTime = 0;
            }
        }
        else if (inMenu){
            inMenu = !processMenuInput(&modeId, gameModes.length);
            inGameModeSettings = !inMenu;
        }
        else if (inGameModeSettings) {
            //------------------------------------------------------------------
            // Game Mode Settings.
            // This is where rotation rule and level is selected before
            // the game starts.
            //------------------------------------------------------------------
            processMenuInput(&idxGameModeSettingOption, 2);

            // level select
            if (IsKeyPressed(config.keyBinds.right) && idxGameModeSettingOption == 0) {
                gameModes.modes[modeId].level += 100;
                if (gameModes.modes[modeId].level > gameModes.modes[modeId].maxLevel) {
                    gameModes.modes[modeId].level = 0;
                }
                gameModeLvl(0, 0, &gameModes.modes[modeId]);
            }
            else if (IsKeyPressed(config.keyBinds.left) && idxGameModeSettingOption == 0) {
                if (gameModes.modes[modeId].level == 0) {
                    gameModes.modes[modeId].level = gameModes.modes[modeId].maxLevel;
                }
                else {
                    gameModes.modes[modeId].level -= 100;
                }
                gameModeLvl(0, 0, &gameModes.modes[modeId]);
            }

            // rotation rule select
            if ((IsKeyPressed(config.keyBinds.right) || IsKeyPressed(config.keyBinds.left)) && idxGameModeSettingOption == 1) {
                if (rotationRule == WORLD) {
                    setRotationRule(0);
                    blockTileset = classicTileset;
                }
                else {
                    setRotationRule(1);
                    blockTileset = worldTileset;
                }
                activePiece.tileset = blockTileset;
            }

            // big mode toggle
            if ((IsKeyPressed(config.keyBinds.right) || IsKeyPressed(config.keyBinds.left)) && idxGameModeSettingOption == 2) {
                rule.bigMode = !rule.bigMode;
                //TODO
                if (rule.bigMode) {
                }
                else {
                }
            }

            if (IsKeyPressed(config.keyBinds.uiSelect)) {
                PlaySound(lineClearSound);
                inGameModeSettings = false;
                startTimer(&delayStartTimer, startCountDown);
            }
            if (IsKeyPressed(KEY_ESCAPE)) {
                inMenu = true;
                inGameModeSettings = false;
            }
        }
        else if (gameOver) {
            rule.invisiblePieces = false;
        }
        else if (pause) {
            bool isSelected = processMenuInput(&idxPauseOption, PAUSE_OPTIONS);
            if (isSelected) {
                if (idxPauseOption == 0) {
                    pause = false;
                    togglePauseGameTimer(&gameModes.modes[modeId].gameTimer);
                }
                else if (idxPauseOption == 1) {
                    resetGame();
                }
                else if (idxPauseOption == 2) {
                    shouldQuit = true;
                }
            }
        }
    }
}

void render(void) {
    SpeedSettings rule = gameModes.modes[modeId].rule;

    BeginTextureMode(screenTexture);
        ClearBackground(BLACK);

        if (currentScreen == TITLE) {
            DrawTexture(titlescreenBG, 0, 0, WHITE);
            drawTitleMenu((Vector2){SCREEN_WIDTH / 2.f, 264}, idxTitleOption);
        }
        else if (currentScreen == OPTIONS) {
            //drawTitleScreenBg(monochromeTileset, SCREEN_WIDTH, SCREEN_HEIGHT, cellSize);
            //drawOptionsMenu(idxOption, screenScales[currentScreenScale]);
        }
        else if (currentScreen == GAME_MODES) {
            Texture2D background = gameModes.modes[modeId].background;
            DrawTexturePro(background, (Rectangle){0,0, background.width, background.height}, (Rectangle){0,0, SCREEN_WIDTH, SCREEN_HEIGHT}, (Vector2){0,0}, 0, WHITE);
            drawPlayFieldBack(playField, gameModes.modes[modeId].boardColor);
            drawPlayField3DEffect(rule,playField);
            drawPiecePreview(activePiece, rule, playField, nextPieceBg);
            drawHeldPiece(activePiece, rule, playField);

            if (!gameOver && !inMenu && !inGameModeSettings && TimerDone(delayStartTimer)) {
                if (rule.creditRoll)
                    drawCredits(creditsTexture, rule.creditRollTimer, playField);

                drawGhostPiece(activePiece, rule, playField);
                drawActivePiece(activePiece, rule, playField, fade);
                drawPlayField(playField, gameModes.modes[modeId]);
                drawStackOutline(playField, rule.invisiblePieces);

                for (int i = 0; i < 4; i++) {
                    if (queuedLines[i] != -1) {
                        lineClearEffect = true;
                    }
                }
            }
            else if (inGameModeSettings) {
                drawGameModeSettings(gameModes.modes[modeId].level, rotationRule, playField, idxGameModeSettingOption);
            }
            else if (inMenu) {
                drawMenu(modeId, gameModes, playField);
            }
            else if (gameOver) {
                drawActivePiece(activePiece, rule, playField, fade);
                drawPlayField(playField, gameModes.modes[modeId]);
                drawGameOverMenu(playField, &gameModes.modes[modeId].rule, altGradeTileset);
            }

            if (pause) {
                drawPauseMenu(idxPauseOption, playField);
            }

            if (!TimerDone(delayStartTimer)) {
                int currentCount = (int)GetElapsed(delayStartTimer);
                //int countDownNumber;
                char readyGo[7];
                Color textColor = BLUE;
                switch(currentCount) {
                    case 2: sprintf(readyGo, "GO!"); textColor = GREEN; break;
                    default: sprintf(readyGo, "Ready?"); break;
                }
                int width = MeasureText(readyGo, 50) / 2;
                DrawText(TextFormat("%s", readyGo),
                        playField.pos.x + ((playField.cellSize * playField.width) / 2.f) - width,
                        playField.pos.y + ((playField.cellSize * playField.height) / 2.f), 50, textColor);

                // Play sound
                if (lastCount != currentCount && currentCount == 0) {
                    lastCount = currentCount;
                    PlaySound(readySound);
                }
                else if (lastCount != currentCount && currentCount == 2) {
                    lastCount = currentCount;
                    PlaySound(goSound);
                }
            }

            drawBorder(frameTileset, playField, gameModes.modes[modeId]);
            drawGrade(gradeTileset, altGradeTileset, rule, playField);
            drawScore(rule, playField);
            drawLevelCount(rule, playField, gameModes.modes[modeId]);
            drawNextGrade(rule, playField);
            drawGameTimer(gameModes.modes[modeId].gameTimer, playField);
            drawKeyPresses(SCREEN_WIDTH, SCREEN_HEIGHT);

            if (lineClearEffect) {
                for (int i = 0; i < 4; i++) {
                    if (queuedLines[i] != -1) {
                        drawLineClearEffect(explosionEffectTileset, playField, explosionCurrentFrame, explosionCurrentLine, queuedLines[i]);
                    }
                }
            }

            //DrawFPS(10, SCREEN_HEIGHT-30);
            DrawText(TextFormat("%d", GetFPS()), 10, SCREEN_HEIGHT - 30, 20, WHITE);
        }

    EndTextureMode();

    BeginDrawing();
        ClearBackground(BLACK);

        float scale = MIN((float)GetScreenWidth()/SCREEN_WIDTH, (float)GetScreenHeight()/SCREEN_HEIGHT);
        float centerWidth = (GetScreenWidth() - ((float)SCREEN_WIDTH*scale))*0.5f;
        float centerHeight = (GetScreenHeight() - ((float)SCREEN_HEIGHT*scale))*0.5f;
        float drawWidth = (float)SCREEN_WIDTH*scale;
        float drawHeight = (float)SCREEN_HEIGHT*scale;


        DrawTexturePro(
            screenTexture.texture,
            (Rectangle){0, 0, screenTexture.texture.width, -screenTexture.texture.height},
            (Rectangle){centerWidth, centerHeight, drawWidth, drawHeight},
            (Vector2){0,0}, 0, WHITE
        );
    EndDrawing();
}

static void cleanUp(void)
{
    unloadPlayField(playField);

    /* Unload Game Modes */
    for (unsigned int i = 0; i < gameModes.length; i++) {
        freeGameMode(&gameModes.modes[i]);
    }
    free(gameModes.modes);

    /* Unload Render Textures */
    UnloadRenderTexture(screenTexture);
    UnloadRenderTexture(creditsTexture);

    /* Unload Textures */
    UnloadTexture(blockTileset);
    UnloadTexture(monochromeTileset);
    UnloadTexture(worldTileset);
    UnloadTexture(classicTileset);
    UnloadTexture(frameTileset);
    UnloadTexture(gradeTileset);
    UnloadTexture(altGradeTileset);
    UnloadTexture(explosionEffectTileset);
    UnloadTexture(nextPieceBg);
    UnloadTexture(titlescreenBG);

    /* Unload Sounds */
    UnloadSound(pieceLockSound);
    UnloadSound(landSound);
    UnloadSound(cheerSound);
    UnloadSound(lineClearSound);
    UnloadSound(moveSound);
    UnloadSound(preRotateSound);
    UnloadSound(selectSound);
    UnloadSound(boardFall);
    UnloadSound(readySound);
    UnloadSound(goSound);
    for (int i = 0; i < 7; i++)
        UnloadSound(pieceSounds[i]);

    CloseWindow();
}

void updateLevel(void)
{
    bool gameStarted = (TimerDone(delayStartTimer) && !inMenu && !inGameModeSettings);
    if (GetGameTimerElapsed(gameModes.modes[modeId].gameTimer) == 0 && gameStarted) {
        startGameTimer(&gameModes.modes[modeId].gameTimer);
    }
    gameModeUpdate(&gameModes.modes[modeId]);
}

void advanceLevel(int lineCount)
{
    gameModeLvl(lineCount, 1, &gameModes.modes[modeId]);
}

void declareGameOver(void)
{
    gameOver = true;
    togglePauseGameTimer(&gameModes.modes[modeId].gameTimer);
}

void resetGame(void)
{
    resetGameMode(&gameModes.modes[modeId]);
    heldPiece = -1;
    inMenu = true;
    inCreditRoll = false;
    pause = false;
    gameOver = false;
    resetGameOverAnim();
    generateInitialPreview(&activePiece, playField, gameModes.modes[modeId].rule);
    for (unsigned int y = 0; y < playField.height; y++) {
        for (unsigned int x = 0; x < playField.width; x++) {
            playField.matrix[y][x].type = 0;
            playField.copy[y][x].type = 0;
            playField.matrix[y][x].tileset = activePiece.tileset;
        }
    }
    playField.pos = (Vector2) {(SCREEN_WIDTH / 2.f) - ((playField.width * playField.cellSize) / 2.f), PLAYFIELD_YSTART};
}
