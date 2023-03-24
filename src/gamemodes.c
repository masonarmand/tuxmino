/*
 * File: gamemodes.c
 * -----------------
 * Functions for loading lua gamemode files
 *
 * Original Author: Mason Armand
 * Contributors:
 * Date Created: Jan 24, 2023
 * Last Modified: Feb 6, 2023
 */

#include "tuxmino.h"

/* function prototypes */
void freeBackgroundList(BackgroundList* bgList);
void appendGameMode(GameModeList* list, GameMode mode);
void initMode(GameMode* mode);
void initBackgroundList(BackgroundList* list, int size);
void registerLuaFunctions(lua_State* lua);

/* getters/setters to be called by lua code */
static int initGameMode(lua_State* lua);
static int setBackground(lua_State* lua);
static int initBackgrounds(lua_State* lua);
static int initBgIdx(lua_State* lua);
static int printc(lua_State* lua);
static int setPieceTint(lua_State* lua);
static int setPlayfieldColor(lua_State* lua);
static int setDAS(lua_State* lua);
static int setARR(lua_State* lua);
static int setARE(lua_State* lua);
static int setLineARE(lua_State* lua);
static int setLockDelay(lua_State* lua);
static int setClearSpeed(lua_State* lua);
static int setGravity(lua_State* lua);
static int setPreview(lua_State* lua);
static int setBagRetry(lua_State* lua);
static int setGradeType(lua_State* lua);
static int setGrade(lua_State* lua);
static int setNextScore(lua_State* lua);
static int setSevenBag(lua_State* lua);
static int setHold(lua_State* lua);
static int setSonicDrop(lua_State* lua);
static int setExtraKicks(lua_State* lua);
static int set3D(lua_State* lua);
static int setDrawNextBg(lua_State* lua);
static int setBig(lua_State* lua);
static int setInvis(lua_State* lua);
static int setDisplayGrade(lua_State* lua);
static int setGhost(lua_State* lua);
static int setCreditRoll(lua_State* lua);
static int getCreditRoll(lua_State* lua);
static int setMaxLevel(lua_State* lua);
static int getMaxLevel(lua_State* lua);
static int setSectionLevel(lua_State* lua);
static int getSectionLevel(lua_State* lua);
static int setLevel(lua_State* lua);
static int getLevel(lua_State* lua);
static int setScore(lua_State* lua);
static int addScore(lua_State* lua);
static int getScore(lua_State* lua);
static int setGoal(lua_State* lua);
static int setSoftFrames(lua_State* lua);
static int getSoftFrames(lua_State* lua);
static int setClearOnCredits(lua_State* lua);
static int setBoneBlocks(lua_State* lua);
static int getElapsedTime(lua_State* lua);


void freeBackgroundList(BackgroundList* bgList)
{
    free(bgList->textures);
}


void freeGameMode(GameMode* mode)
{
    //free(mode->filename);
    free(mode->name);
    freeBackgroundList(&mode->bgList);
    lua_close(mode->interpreter);
}


void appendGameMode(GameModeList* list, GameMode mode)
{
    int newSize = list->length + 1;
    GameMode* newAddr = (GameMode*) realloc(list->modes, (newSize * sizeof(GameMode)));

    if (newAddr == NULL) {
        printf("ERROR: Unable to realloc memory for game modes array\n");
    } else {
        int idx = newSize - 1;
        newAddr[idx] = mode;
        list->modes = newAddr;
        list->length = newSize;
    }
}


void initMode(GameMode* mode)
{
    /* init gamemode lua interpreter */
    mode->interpreter = luaL_newstate();
    luaL_openlibs(mode->interpreter);

    int error = luaL_loadfile(mode->interpreter, mode->filename);
    if (error == 0) {
        printf("LUA: Successfully loaded %s\n", mode->filename);

        /* priming pcall */
        error = lua_pcall(mode->interpreter, 0, 0, 0);
        if (error != 0) {
            printf(
                "LUA: [ERROR] [%i] attempting to call function: '%s'\n",
                error,
                lua_tostring(mode->interpreter, -1)
            );
        }

        lua_pushlightuserdata(mode->interpreter, mode);
        lua_setglobal(mode->interpreter, "mode");
        registerLuaFunctions(mode->interpreter);

        /* Push start() function name onto the stack */
        lua_getglobal(mode->interpreter, "start");
        printf(
            "LUA: called lua_getglobal. lua stack height is now %d\n",
            lua_gettop(mode->interpreter)
        );

        /* call start() */
        error = lua_pcall(mode->interpreter, 0, 0, 0);
        if (error != 0) {
            printf(
                "LUA: [ERROR] [%i] attempting to call function: '%s'\n",
                error,
                lua_tostring(mode->interpreter, -1)
            );
        }
        lua_pop(mode->interpreter, 0);
        printf(
            "LUA: successfully called start(). lua stack height is now %d\n",
            lua_gettop(mode->interpreter)
        );
    }
    else {
        printf(
            "LUA: [ERROR] loading %s Error code: %s\n",
            mode->filename,
            lua_tostring(mode->interpreter, -1)
        );
    }

    resetGameMode(mode);
}


void initBackgroundList(BackgroundList* list, int size)
{
    if (size >= 0) {
        list->textures = (Texture2D *) calloc(size, sizeof(Texture2D));
        if (list->textures == NULL) {
            printf("LUA: [ERROR] Unable to allocate memory for backgroundList\n");
        }
        else {
            list->length = size;
        }
    }
    else {
        printf("LUA: [ERROR] Unable to allocate memory for backgroundList with negative size\n");
    }
}


void gameModeUpdate(GameMode* mode)
{
    lua_getglobal(mode->interpreter, "update");

    if (lua_pcall(mode->interpreter, 0, 0, 0) != 0) {
        printf("LUA: [ERROR] in update(): %s\n", lua_tostring(mode->interpreter, -1));
    }
    lua_pop(mode->interpreter, 0);
}


void gameModeLvl(int lineCount, int amount, GameMode* mode)
{
    lua_getglobal(mode->interpreter, "advanceLevel");
    lua_pushnumber(mode->interpreter, amount);
    lua_pushnumber(mode->interpreter, lineCount);
    if (lua_pcall(mode->interpreter, 2, 0, 0) != 0) {
        printf("LUA: [ERROR] in advanceLevel(): %s\n", lua_tostring(mode->interpreter, -1));
    }
    lua_pop(mode->interpreter, 0);
}


void resetGameMode(GameMode* mode)
{
    mode->level = 0;
    mode->rule.score = 0;
    mode->rule.creditRoll = false;
    mode->rule.grade = 0;
    mode->gameTimer.paused = false;
    mode->gameTimer.pauseTime = 0;
    resetGameTimer(&mode->gameTimer);
    resetTimer(&mode->rule.creditRollTimer);
    gameModeLvl(0, 0, mode);

    lua_getglobal(mode->interpreter, "reset");
    if (lua_pcall(mode->interpreter, 0, 0, 0) != 0) {
        printf("LUA: [ERROR] reset(): %s\n", lua_tostring(mode->interpreter, -1));
    }
}


void loadGameModes(GameModeList* list)
{
    FilePathList luaFiles;
    luaFiles = LoadDirectoryFiles("gamemodes");

    for (size_t i = 0; i < luaFiles.count; i++) {
        GameMode mode;

        mode.filename = luaFiles.paths[i];
        mode.bgList.length = 0;

        appendGameMode(list, mode);
    }

    for (size_t i = 0; i < list->length; i++) {
        initMode(&list->modes[i]);
    }

    UnloadDirectoryFiles(luaFiles);
}


void registerLuaFunctions(lua_State* lua)
{
    lua_register(lua, "initGameMode", initGameMode);
    lua_register(lua, "setBackground", setBackground);
    lua_register(lua, "initBackgrounds", initBackgrounds);
    lua_register(lua, "initBgIdx", initBgIdx);
    lua_register(lua, "printc", printc);
    lua_register(lua, "setDAS", setDAS);
    lua_register(lua, "setARR", setARR);
    lua_register(lua, "setARE", setARE);
    lua_register(lua, "setLineARE", setLineARE);
    lua_register(lua, "setLockDelay", setLockDelay);
    lua_register(lua, "setClearSpeed", setClearSpeed);
    lua_register(lua, "setGravity", setGravity);
    lua_register(lua, "setPreview", setPreview);
    lua_register(lua, "setBagRetry", setBagRetry);
    lua_register(lua, "setGradeType", setGradeType);
    lua_register(lua, "setGrade", setGrade);
    lua_register(lua, "setNextScore", setNextScore);
    lua_register(lua, "setSevenBag", setSevenBag);
    lua_register(lua, "setHold", setHold);
    lua_register(lua, "setSonicDrop", setSonicDrop);
    lua_register(lua, "setExtraKicks", setExtraKicks);
    lua_register(lua, "set3D", set3D);
    lua_register(lua, "setDrawNextBg", setDrawNextBg);
    lua_register(lua, "setBig", setBig);
    lua_register(lua, "setInvis", setInvis);
    lua_register(lua, "setDisplayGrade", setDisplayGrade);
    lua_register(lua, "setGhost", setGhost);
    lua_register(lua, "setCreditRoll", setCreditRoll);
    lua_register(lua, "getCreditRoll", getCreditRoll);
    lua_register(lua, "setMaxLevel", setMaxLevel);
    lua_register(lua, "getMaxLevel", getMaxLevel);
    lua_register(lua, "setSectionLevel", setSectionLevel);
    lua_register(lua, "getSectionLevel", getSectionLevel);
    lua_register(lua, "setLevel", setLevel);
    lua_register(lua, "getLevel", getLevel);
    lua_register(lua, "setScore", setScore);
    lua_register(lua, "addScore", addScore);
    lua_register(lua, "getScore", getScore);
    lua_register(lua, "setGoal", setGoal);
    lua_register(lua, "setSoftFrames", setSoftFrames);
    lua_register(lua, "getSoftFrames", getSoftFrames);
    lua_register(lua, "setPieceTint", setPieceTint);
    lua_register(lua, "setPlayfieldColor", setPlayfieldColor);
    lua_register(lua, "setClearOnCredits", setClearOnCredits);
    lua_register(lua, "setBoneBlocks", setBoneBlocks);
    lua_register(lua, "getElapsedTime", getElapsedTime);
}


static int initGameMode(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    char* strName = (char*) lua_tostring(lua, 1);
    mode->name = malloc(strlen(strName) + 1);
    strcpy(mode->name, (char *) lua_tostring(lua, 1));

    mode->frameColor.r = (int)lua_tonumber(lua, 2);
    mode->frameColor.g = (int)lua_tonumber(lua, 3);
    mode->frameColor.b = (int)lua_tonumber(lua, 4);
    mode->frameColor.a = 255;

    mode->pieceTint = WHITE;
    mode->boardColor = BLACK;
    mode->rule.score = 0;
    mode->rule.nextScore = 0;
    mode->rule.linesSoftDropped = 0;
    mode->rule.grade = 0;
    mode->rule.score = 0;
    mode->rule.nextScore = 0;
    mode->rule.creditRoll = false;
    mode->level = 0;
    mode->sectionLevel = 0;
    mode->maxLevel = 0;
    mode->boneBlocks = false;
    resetTimer(&mode->rule.creditRollTimer);

    return 0;
}


static int setBackground(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    unsigned int idx = (unsigned int)lua_tonumber(lua, 1);
    if (idx < mode->bgList.length && mode->bgList.length != 0) {
        mode->background = mode->bgList.textures[idx];
    }
    else {
        printf(
            "LUA: [ERROR] Invalid set index on background. \
            BackgroundList is of size %d, index was %d\n",
            mode->bgList.length, idx
        );
    }
    return 0;
}


static int initBackgrounds(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    int size = (int) lua_tonumber(lua, 1);
    initBackgroundList(&mode->bgList, size);
    return 0;
}


static int initBgIdx(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    unsigned int idx = (int) lua_tonumber(lua, 1);
    if (idx < mode->bgList.length) {
        char* filename = (char*) lua_tostring(lua, 2);
        mode->bgList.textures[idx] = LoadTexture(filename);
    }
    return 0;
}


static int printc(lua_State* lua)
{
    printf("LUA: [DEBUG] %s\n", lua_tostring(lua, 1));
    return 0;
}


static int setPieceTint(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);
    unsigned char r = (unsigned char) lua_tonumber(lua, 1);
    unsigned char g = (unsigned char) lua_tonumber(lua, 2);
    unsigned char b = (unsigned char) lua_tonumber(lua, 3);
    unsigned char a = (unsigned char) lua_tonumber(lua, 4);
    mode->pieceTint = (Color) {r, g, b, a};
    return 0;
}

static int setPlayfieldColor(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);
    unsigned char r = (unsigned char) lua_tonumber(lua, 1);
    unsigned char g = (unsigned char) lua_tonumber(lua, 2);
    unsigned char b = (unsigned char) lua_tonumber(lua, 3);
    unsigned char a = (unsigned char) lua_tonumber(lua, 4);
    mode->boardColor = (Color) {r, g, b, a};
    return 0;
}


static int setDAS(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.delayedAutoShift = framesToMilliseconds((int)lua_tonumber(lua, 1));
    return 0;
}


static int setARR(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.autoRepeatRate = framesToMilliseconds((int)lua_tonumber(lua, 1));
    return 0;
}


static int setARE(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.appearanceDelay = framesToMilliseconds((int)lua_tonumber(lua, 1));
    return 0;
}


static int setLineARE(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.lineAppearanceDelay = framesToMilliseconds((int)lua_tonumber(lua, 1));
    return 0;
}


static int setLockDelay(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.lockDelay = framesToMilliseconds((int)lua_tonumber(lua, 1));
    return 0;
}


static int setClearSpeed(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.lineClearSpeed = framesToMilliseconds((int)lua_tonumber(lua, 1));
    return 0;
}


static int setGravity(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.tickSpeed = gravityLevelToMilliseconds((int)lua_tonumber(lua, 1));
    return 0;
}


static int setPreview(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.piecePreviewCount = (int)lua_tonumber(lua, 1);
    return 0;
}


static int setBagRetry(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.bagRetry = (int)lua_tonumber(lua, 1);
    return 0;
}


static int setGradeType(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.gradeType = (int)lua_tonumber(lua, 1);
    return 0;
}


static int setGrade(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.grade = (int)lua_tonumber(lua, 1);
    return 0;
}


static int setNextScore(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.nextScore = (int)lua_tonumber(lua, 1);
    return 0;
}


static int setClearOnCredits(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.clearOnCredits = (bool)lua_toboolean(lua, 1);
    return 0;
}


static int setSevenBag(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.sevenBag = (bool)lua_toboolean(lua, 1);
    return 0;
}


static int setHold(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.holdEnabled = (bool)lua_toboolean(lua, 1);
    return 0;
}


static int setSonicDrop(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.sonicDropEnabled = (bool)lua_toboolean(lua, 1);
    return 0;
}


static int setExtraKicks(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.extraKicks = (bool)lua_toboolean(lua, 1);
    return 0;
}


static int set3D(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.effect3D = (bool)lua_toboolean(lua, 1);
    return 0;
}


static int setDrawNextBg(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.drawNextPieceBG = (bool)lua_toboolean(lua, 1);
    return 0;
}


static int setBig(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.bigMode = (bool)lua_toboolean(lua, 1);
    return 0;
}


static int setInvis(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.invisiblePieces = (bool)lua_toboolean(lua, 1);
    return 0;
}


static int setDisplayGrade(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.displayGrade = (bool)lua_toboolean(lua, 1);
    return 0;
}


static int setGhost(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.showGhost = (bool)lua_toboolean(lua, 1);
    return 0;
}


static int setCreditRoll(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    bool doCreditRoll = (bool)lua_toboolean(lua, 1);

    if (doCreditRoll && mode->rule.creditRollTimer.startTime == 0)
        startTimer(&mode->rule.creditRollTimer, 42);

    mode->rule.creditRoll = doCreditRoll;
    return 0;
}


static int getCreditRoll(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    lua_pushboolean(lua, mode->rule.creditRoll);
    return 1;
}


static int setMaxLevel(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->maxLevel = (int)lua_tonumber(lua, 1);
    return 0;
}


static int getMaxLevel(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    lua_pushnumber(lua, mode->maxLevel);
    return 1;
}


static int setSectionLevel(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->sectionLevel = (int)lua_tonumber(lua, 1);
    return 0;
}


static int getSectionLevel(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    lua_pushnumber(lua, mode->sectionLevel);
    return 1;
}


static int setLevel(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->level = (int)lua_tonumber(lua, 1);
    return 0;
}


static int getLevel(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    lua_pushnumber(lua, mode->level);
    return 1;
}


static int setScore(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.score = (int)lua_tonumber(lua, 1);
    return 0;
}


static int addScore(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.score += (int)lua_tonumber(lua, 1);
    return 0;
}


static int getScore(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    lua_pushnumber(lua, mode->rule.score);
    return 1;
}


static int setGoal(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.nextScore = (int)lua_tonumber(lua, 1);
    return 0;
}


static int setSoftFrames(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.linesSoftDropped = (int)lua_tonumber(lua, 1);
    return 0;
}


static int getSoftFrames(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    lua_pushnumber(lua, mode->rule.linesSoftDropped);
    return 1;
}

static int setBoneBlocks(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->boneBlocks = (bool) lua_toboolean(lua, 1);
    return 1;
}


static int getElapsedTime(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    lua_pushnumber(lua, GetGameTimerElapsed(mode->gameTimer));
    return 1;
}
