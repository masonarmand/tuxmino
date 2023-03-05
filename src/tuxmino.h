/*
 * File: tuxmino.h
 * ---------------
 * Original Author: Mason Armand
 * Contributors:
 * Date Created: Jan 24, 2023 (previously multiple header files were used)
 * Last Modified: Feb 14, 2023
 */

#ifndef TUXMINO_H
#define TUXMINO_H

#include <raylib.h>
#include <lua5.4/lua.h>
#include <lua5.4/lualib.h>
#include <lua5.4/lauxlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ini.h"

// Macros
#define MIN(a, b) ((a)<(b)? (a) : (b))

// -----------------------------------------------------------------------------
// enums
// -----------------------------------------------------------------------------

enum gameScreens {TITLE, GAME_MODES, OPTIONS};
enum rotationSystems {CLASSIC, WORLD};

// -----------------------------------------------------------------------------
// defined structs
// -----------------------------------------------------------------------------

// config structs
typedef struct {
    unsigned int displayWidth;
    unsigned int displayHeight;
    unsigned int fps;
} Settings;

typedef struct {
    int ccw;
    int ccwAlt;
    int cw;
    int sonicDrop;
    int swapHold;
    int left;
    int right;
    int down;
    int up;
    int uiSelect;
} KeyBinds;

typedef struct {
    Settings settings;
    KeyBinds KeyBinds;
} Config;



typedef struct {
    char** str;
    int size;
} CreditsList;

typedef struct {
  double startTime;
  double lifeTime;
} Timer;

typedef struct {
  double startTime;
  double stoppedTime;
  double pauseTime;
  bool paused;
} GameTimer;

typedef struct {
    int type;
    bool visible;
    Texture2D tileset;
} Block;

typedef struct {
    unsigned int width;
    unsigned int height;
    float cellSize;
    Texture2D blankBlockTexture;
    Vector2 pos;
    Block** matrix;
    Block** copy;
} PlayField;

typedef struct {
    Vector2 position;
    Texture2D currentTileset;
    Texture2D tileset;
    Texture2D boneBlocks;
    unsigned int pieceIndex;
    unsigned int rotIndex;
    bool locked;
} Piece;

// rule/speed settings
typedef struct {
    float delayedAutoShift;
    float autoRepeatRate;
    float appearanceDelay;
    float lineAppearanceDelay;
    float lockDelay;
    float lineClearSpeed;
    double tickSpeed;

    unsigned int piecePreviewCount;
    unsigned int bagRetry;
    unsigned int gradeType; // 0 = grade backgrounds, 1 = text only
    bool sevenBag;
    bool holdEnabled;
    bool sonicDropEnabled;
    bool extraKicks;
    bool effect3D;
    bool drawNextPieceBG;
    bool bigMode;

    unsigned int grade;
    unsigned int score;
    unsigned int nextScore;
    unsigned int combo;
    unsigned int prevComboScore;
    unsigned int linesSoftDropped;
    unsigned int scoringType;

    bool invisiblePieces;
    bool displayGrade;
    bool showGhost;
    bool creditRoll;
    bool clearOnCredits;

    Timer creditRollTimer;

} SpeedSettings;

typedef struct {
    Texture2D* textures;
    unsigned int length;
} BackgroundList;

typedef struct {
    char* filename;
    char* name;
    BackgroundList bgList;
    Texture2D classicTex;
    Texture2D worldTex;
    Texture2D background;
    Color frameColor;
    Color boardColor;
    Color pieceTint;
    SpeedSettings rule;

    bool boneBlocks;

    unsigned int level;
    unsigned int sectionLevel;
    unsigned int maxLevel;

    lua_State* interpreter;
} GameMode;

typedef struct {
    GameMode* modes;
    unsigned int length;
} GameModeList;


// -----------------------------------------------------------------------------
// Global Externs
// -----------------------------------------------------------------------------

/*
 * main.c externs
 */
extern Config config;
extern Sound pieceLockSound;
extern Sound landSound;
extern Sound cheerSound;
extern Sound lineClearSound;
extern Sound moveSound;
extern Sound preRotateSound;
extern Sound selectSound;
extern Sound boardFall;
extern Sound pieceSounds[7];

/*
 * engine.c externs
 */
extern int upcomingPieces[3];
extern int queuedLines[4];
extern int heldPiece;

/*
 * piece.c externs
 */
extern int pieces[7][4][4][4];
extern enum rotationSystems rotationRule;


// ---------------------
// prototyped functions
// ---------------------

/*
 * record.c functions
 */
void recordInputs(void);
void exitRecording(void);
void initRecording(void);

/*
 * config.c functions
 */
int handler(void* config, const char* section, const char* name, const char* value);

/*
 * credits.c functions
 */
RenderTexture2D renderCreditsTexture(PlayField playField);

/*
 * engine.c functions
 */
void spawnQueuedPiece(Piece* activePiece, PlayField playField, SpeedSettings timings);
void moveQueuedLinesDown(PlayField playField);
void checkIfAtBottom(Piece* activePiece, PlayField playField, SpeedSettings timings);
void processInput(Piece* activePiece, PlayField playField, SpeedSettings* timings);
void moveDown(Piece* activePiece, PlayField playField, SpeedSettings timings);
void generateInitialPreview(Piece* activePiece, PlayField playField, SpeedSettings rule);
void fillRow(int row, int typeID, PlayField playField);
void unloadPlayField(PlayField playField);
void setPiece(int piece[4][4]);
bool canMove(Vector2 position, Piece* activePiece, PlayField playField, int dirX, int dirY);
bool canDrawFlash(void);
bool processMenuInput(unsigned int* menuIndex, unsigned int max);
bool canDrawPiece(void);
PlayField initPlayField(int width, int height, Texture blockTileset);
Vector2 getFinalPos(Piece* activePiece, PlayField playField);

/*
 * draw.c functions
 */
void drawTitleMenu(Vector2 pos, int idxOption);
void drawMenu(int selectedGameType, GameModeList gameModes, PlayField playField);
void drawKeyPresses(int screenWidth, int ScreenHeight);
void drawCredits(RenderTexture2D creditsTexture, Timer creditRollTimer, PlayField playField);
void drawBorder(Texture2D borderTileset, PlayField playField, GameMode mode);
void drawGrade(Texture2D gradeTexture, Texture2D altGradeTexture, SpeedSettings rule, PlayField playField);
void drawGradeEx(Texture2D gradeTexture, int grade, Vector2 gradePos);
void drawScore(SpeedSettings rule, PlayField playField);
void drawLevelCount(SpeedSettings rule, PlayField playField, GameMode mode);
void drawNextGrade(SpeedSettings rule, PlayField playField);
void drawLineClearEffect(Texture2D explosionTileset, PlayField playField, int currentFrame, int currentLine, int row);
void drawPiecePreview(Piece activePiece, SpeedSettings rule, PlayField playField, Texture2D nextBg);
void drawHeldPiece(Piece activePiece, SpeedSettings rule, PlayField playField);
void drawActivePiece(Piece activePiece, SpeedSettings rule, PlayField playField, Color color);
void drawGhostPiece(Piece activePiece, SpeedSettings rule, PlayField playField);
void drawPlayFieldBack(PlayField playField, Color color);
void drawPlayField3DEffect(SpeedSettings rule, PlayField playField);
void drawPlayField(PlayField playField, GameMode mode);
void drawStackOutline(PlayField playField, bool isInvisible);
void drawGameModeSettings(int level, enum rotationSystems rule, PlayField playField, int idxOption);
void drawPauseMenu(int idxOption, PlayField playField);
void drawGameOverMenu(PlayField playField, SpeedSettings* rule, Texture2D gradeTileset);
void resetGameOverAnim(void);
void drawGameTimer(GameTimer timer, PlayField playField);

/*
 * main.c functions
 */
void advanceLevel(int lineCount);
void declareGameOver(void);
void resetGame(void);

/*
 * piece.c functions
 */
void setRotationRule(int type);

/*
 * timer.c functions
 */
void startTimer(Timer *timer, double lifetime);
void resetTimer(Timer *timer);
bool TimerDone(Timer timer);
void startGameTimer(GameTimer *timer);
void resetGameTimer(GameTimer *timer);
void togglePauseGameTimer(GameTimer *timer);
double GetElapsed(Timer timer);
double GetGameTimerElapsed(GameTimer timer);

/*
 * timings.c functions
 */
double framesToMilliseconds(int frames);
double gravityLevelToMilliseconds(int gravity);

/*
 * gamemodes.c functions
 */
void freeGameMode(GameMode* mode);
void loadGameModes(GameModeList* list);
void gameModeUpdate(GameMode* mode);
void gameModeLvl(int lineCount, int level, int amount, GameMode* mode);
void resetGameMode(GameMode* mode);

#endif
