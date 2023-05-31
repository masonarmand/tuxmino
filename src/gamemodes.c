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
    GameMode* newAddr = realloc(list->modes, (newSize * sizeof(GameMode)));

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
    lua_register(lua, "setSoftFrames", setSoftFrames);
    lua_register(lua, "getSoftFrames", getSoftFrames);
    lua_register(lua, "setPieceTint", setPieceTint);
    lua_register(lua, "setPlayfieldColor", setPlayfieldColor);
    lua_register(lua, "setClearOnCredits", setClearOnCredits);
    lua_register(lua, "setBoneBlocks", setBoneBlocks);
    lua_register(lua, "getElapsedTime", getElapsedTime);
}

/*
 * lua function
 * @brief Initializes the Gamemode.
 * This function must be called first when creating a gamemode.
 *
 * @param name Name of the game mode
 * @param r Red value of the playfield frame
 * @param g Green value of the playfield frame
 * @param b Blue value of the playfield frame
 * @returns nothing
 * @usage initGameMode(name, r, g, b)
 */
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

/*
 * lua function
 * @brief Sets the current background
 * This function changes the background of the game mode to the specified index.
 * If the index is out of bounds, an error message is printed.
 *
 * @param idx The index of the background to set
 * @returns nothing
 * @usage setBackground(idx)
 */
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

/*
 * lua function
 * @brief Initializes an array of backgrounds.
 * This function creates an array of backgrounds given a size.
 *
 * @param size Number of backgrounds
 * @returns nothing
 * @usage initBackgrounds(size)
 */
static int initBackgrounds(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    int size = (int) lua_tonumber(lua, 1);
    initBackgroundList(&mode->bgList, size);
    return 0;
}

/*
 * lua function
 * @brief Load a background from file.
 * This function initializes an index in the backgrounds array
 * and loads an image from a filename.
 *
 * @param idx Background index to initialize
 * @param filename Name of the image file
 * @returns nothing
 * @usage initBgIdx(idx, filename)
 */
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

/*
 * lua function
 * @brief Print a message to the console
 *
 * @param message The message to print
 * @returns nothing
 * @usage printc(message)
 */
static int printc(lua_State* lua)
{
    printf("LUA: [DEBUG] %s\n", lua_tostring(lua, 1));
    return 0;
}

/*
 * lua function
 * @brief Set the color of the piece tint
 * The piece will tint to the specified color when it locks to a grid.
 *
 * @param r Red value
 * @param g Green value
 * @param b Blue value
 * @returns nothing
 * @usage setPieceTinit(r, g, b)
 */
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

/*
 * lua function
 * @brief Set the color of the playfield background
 *
 * @param r Red value
 * @param g Green value
 * @param b Blue value
 * @param a Alpha value
 * @returns nothing
 * @usage setPlayfieldColor(r, g, b, a)
 */
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

/*
 * lua function
 * @brief Set the Delayed Auto Shift value
 * DAS is the delay time before a piece starts to auto-shift
 * left or right when a directional key is held down.
 * The setDAS() function configures this delay time.
 *
 * @param frames Number of pseudo frames
 * @returns nothing
 * @usage setDAS(frames)
 */
static int setDAS(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.delayedAutoShift = framesToMilliseconds((int)lua_tonumber(lua, 1));
    return 0;
}

/*
 * lua function
 * @brief Set the Auto Repeat Rate value
 * ARR is the speed at which Tetriminos continue to move left
 * or right when a directional key is held down.
 * The setARR() function configures this speed.
 *
 * @param frames Number of pseudo frames
 * @returns nothing
 * @usage setARR(frames)
 */
static int setARR(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.autoRepeatRate = framesToMilliseconds((int)lua_tonumber(lua, 1));
    return 0;
}

/*
 * lua function
 * @brief Set the Auto Repeat Entry value
 * ARE is the delay time before a new Tetrimino enters the game field
 * after the previous one has been placed.
 * The setARE() function configures this delay time.
 *
 * @param frames Number of pseudo frames
 * @returns nothing
 * @usage setARE(frames)
 */
static int setARE(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.appearanceDelay = framesToMilliseconds((int)lua_tonumber(lua, 1));
    return 0;
}

/*
 * lua function
 * @brief Set the Line Auto Repeat Entry value
 * LineARE is the delay time before a new Tetrimino enters the game field
 * after a line has been cleared.
 * The setLineARE() function configures this delay time.
 *
 * @param frames Number of pseudo frames
 * @returns nothing
 * @usage setLineARE(frames)
 */
static int setLineARE(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.lineAppearanceDelay = framesToMilliseconds((int)lua_tonumber(lua, 1));
    return 0;
}

/*
 * lua function
 * @brief Set the Lock Delay value
 * Lock Delay is the amount of time a piece can be manipulated before it
 * locks into place after it lands.
 * The setLockDelay() function configures this delay time.
 *
 * @param frames Number of pseudo frames
 * @returns nothing
 * @usage setLockDelay(frames)
 */
static int setLockDelay(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.lockDelay = framesToMilliseconds((int)lua_tonumber(lua, 1));
    return 0;
}

/*
 * lua function
 * @brief Set the line clear speed
 * Clear Speed is the speed at which cleared lines disappear from the game field.
 * The setClearSpeed() function configures this speed.
 *
 * @param frames Number of pseudo frames
 * @returns nothing
 * @usage setClearSpeed(frames)
 */

static int setClearSpeed(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.lineClearSpeed = framesToMilliseconds((int)lua_tonumber(lua, 1));
    return 0;
}

/*
 * lua function
 * @brief Set the gravity level
 * Gravity Level determines the speed at which Tetriminos fall, measured in 1/256 of a G.
 * 5120 represents 20G. The setGravity() function configures this gravity level.
 *
 * @param gravityLevel Number representing gravity level
 * @returns nothing
 * @usage setGravity(gravityLevel)
 */
static int setGravity(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.tickSpeed = gravityLevelToMilliseconds((int)lua_tonumber(lua, 1));
    return 0;
}

/*
 * lua function
 * @brief Set the number of preview pieces
 * Preview Pieces are the next pieces that will enter the game field.
 * The setPreview() function configures the number of these preview pieces.
 *
 * @param amount Number of preview pieces
 * @returns nothing
 * @usage setPreview(amount)
 */

static int setPreview(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.piecePreviewCount = (int)lua_tonumber(lua, 1);
    return 0;
}

/*
 * lua function
 * @brief Set the number of bag retry attempts
 * Bag Retry refers to the number of times the random number generator will
 * try to generate a piece not currently in the "bag" (the set of all pieces).
 * The setBagRetry() function configures this amount of retry attempts.
 *
 * @param amount Number of retry attempts
 * @returns nothing
 * @usage setBagRetry(amount)
 */
static int setBagRetry(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.bagRetry = (int)lua_tonumber(lua, 1);
    return 0;
}

/*
 * lua function
 * @brief Set the type of grade display
 * Grade Type changes how the grade is displayed in the game.
 * The setGradeType() function configures this grade type.
 *
 * @param number Number representing grade type
 * @returns nothing
 * @usage setGradeType(number)
 */
static int setGradeType(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.gradeType = (int)lua_tonumber(lua, 1);
    return 0;
}

/*
 * lua function
 * @brief Set the game grade
 * Grade is a numerical value determining player's performance, with 18 representing a grand master.
 * The setGrade() function sets this grade.
 *
 * @param grade Number representing grade
 * @returns nothing
 * @usage setGrade(grade)
 */
static int setGrade(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.grade = (int)lua_tonumber(lua, 1);
    return 0;
}

/*
 * lua function
 * @brief Set the score goal for the next grade
 * Next Score is the score that a player needs to reach to achieve the next grade.
 * The setNextScore() function configures this score goal.
 *
 * @param score Score goal for the next grade
 * @returns nothing
 * @usage setNextScore(score)
 */
static int setNextScore(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.nextScore = (int)lua_tonumber(lua, 1);
    return 0;
}

/*
 * lua function
 * @brief Set whether to clear the playfield when credits start rolling
 * Clear On Credits is a feature that clears the game field when the credits start.
 * The setClearOnCredits() function enables or disables this feature.
 *
 * @param doClear Boolean indicating whether to clear or not
 * @returns nothing
 * @usage setClearOnCredits(doClear)
 */
static int setClearOnCredits(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.clearOnCredits = (bool)lua_toboolean(lua, 1);
    return 0;
}

/*
 * lua function
 * @brief Set whether to enable seven bag system
 * Seven Bag system ensures that all seven tetriminos are dealt in a 'bag' before any new piece is repeated.
 * The setSevenBag() function enables or disables this feature.
 *
 * @param enableSevenBag Boolean indicating whether to enable seven bag or not
 * @returns nothing
 * @usage setSevenBag(enableSevenBag)
 */
static int setSevenBag(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.sevenBag = (bool)lua_toboolean(lua, 1);
    return 0;
}

/*
 * lua function
 * @brief Set whether to enable hold feature
 * Hold feature allows a player to save a Tetrimino for later use.
 * The setHold() function enables or disables this feature.
 *
 * @param enableHold Boolean indicating whether to enable hold or not
 * @returns nothing
 * @usage setHold(enableHold)
 */
static int setHold(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.holdEnabled = (bool)lua_toboolean(lua, 1);
    return 0;
}

/*
 * lua function
 * @brief Set whether to enable Sonic Drop feature
 * Sonic Drop is a feature that lets a player hard drop a Tetrimino instantly,
 * making it lock in place as soon as it lands.
 * The setSonicDrop() function enables or disables this feature.
 *
 * @param enableSonicDrop Boolean indicating whether to enable Sonic Drop or not
 * @returns nothing
 * @usage setSonicDrop(enableSonicDrop)
 */
static int setSonicDrop(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.sonicDropEnabled = (bool)lua_toboolean(lua, 1);
    return 0;
}

/*
 * lua function
 * @brief Set whether to enable extra kicks for I and T pieces
 * Extra Kicks allows I and T Tetriminos to perform additional rotational
 * maneuvers ("kicks") when they are blocked.
 * The setExtraKicks() function enables or disables this feature.
 *
 * @param enableExtraKicks Boolean indicating whether to enable Extra Kicks or not
 * @returns nothing
 * @usage setExtraKicks(enableExtraKicks)
 */
static int setExtraKicks(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.extraKicks = (bool)lua_toboolean(lua, 1);
    return 0;
}

/*
 * lua function
 * @brief Set whether to enable pseudo 3D effect on Tetriminos
 * Pseudo 3D effect adds a three-dimensional visual effect to Tetriminos.
 * The set3D() function enables or disables this feature.
 *
 * @param enable3D Boolean indicating whether to enable pseudo 3D effect or not
 * @returns nothing
 * @usage set3D(enable3D)
 */
static int set3D(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.effect3D = (bool)lua_toboolean(lua, 1);
    return 0;
}

/*
 * lua function
 * @brief Set whether to draw a black backdrop behind the piece preview
 * Draw Next Bg determines whether a black backdrop is rendered behind the Tetrimino preview.
 * The setDrawNextBg() function enables or disables this feature.
 *
 * @param drawNextBg Boolean indicating whether to draw a black backdrop or not
 * @returns nothing
 * @usage setDrawNextBg(drawNextBg)
 */
static int setDrawNextBg(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.drawNextPieceBG = (bool)lua_toboolean(lua, 1);
    return 0;
}

/*
 * lua function
 * @brief Set whether to enable Big Mode
 * Big Mode increases the size of the Tetriminos, typically making them twice as large.
 * The setBig() function enables or disables this feature.
 *
 * @param enableBig Boolean indicating whether to enable Big Mode or not
 * @returns nothing
 * @usage setBig(enableBig)
 */
static int setBig(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.bigMode = (bool)lua_toboolean(lua, 1);
    return 0;
}

/*
 * lua function
 * @brief Set whether to make the Tetriminos appear invisible
 * Invisibility Mode makes the Tetriminos invisible after they are locked in place.
 * The setInvis() function enables or disables this feature.
 *
 * @param enableInvis Boolean indicating whether to enable Invisibility Mode or not
 * @returns nothing
 * @usage setInvis(enableInvis)
 */
static int setInvis(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.invisiblePieces = (bool)lua_toboolean(lua, 1);
    return 0;
}

/*
 * lua function
 * @brief Set whether to display the player's grade
 * Display Grade determines whether the player's current grade is shown.
 * The setDisplayGrade() function enables or disables this feature.
 *
 * @param displayGrade Boolean indicating whether to display the grade or not
 * @returns nothing
 * @usage setDisplayGrade(displayGrade)
 */
static int setDisplayGrade(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.displayGrade = (bool)lua_toboolean(lua, 1);
    return 0;
}

/*
 * lua function
 * @brief Set whether to show ghost preview
 * Ghost Preview shows the position where the currently controlled Tetrimino will land.
 * The setGhost() function enables or disables this feature.
 *
 * @param enableGhost Boolean indicating whether to enable Ghost Preview or not
 * @returns nothing
 * @usage setGhost(enableGhost)
 */
static int setGhost(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.showGhost = (bool)lua_toboolean(lua, 1);
    return 0;
}

/*
 * lua function
 * @brief Set whether to start playing the credit roll
 * Credit Roll is a feature that starts the end credits of the game.
 * The setCreditRoll() function enables or disables this feature.
 *
 * @param enableCreditRoll Boolean indicating whether to start Credit Roll or not
 * @returns nothing
 * @usage setCreditRoll(enableCreditRoll)
 */
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

/*
 * lua function
 * @brief Get the status of the credit roll
 * This function returns whether or not the Credit Roll is currently playing.
 *
 * @returns Boolean indicating whether Credit Roll is playing or not
 * @usage getCreditRoll()
 */
static int getCreditRoll(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    lua_pushboolean(lua, mode->rule.creditRoll);
    return 1;
}

/*
 * lua function
 * @brief Set the maximum level of the game mode
 * The setMaxLevel() function sets the highest selectable level in the current game mode.
 *
 * @param maxLevel Integer representing the maximum level
 * @returns nothing
 * @usage setMaxLevel(maxLevel)
 */
static int setMaxLevel(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->maxLevel = (int)lua_tonumber(lua, 1);
    return 0;
}

/*
 * lua function
 * @brief Get the maximum level of the game mode
 * This function returns the highest selectable level in the current game mode.
 *
 * @returns Integer representing the maximum level
 * @usage getMaxLevel()
 */

static int getMaxLevel(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    lua_pushnumber(lua, mode->maxLevel);
    return 1;
}

/*
 * lua function
 * @brief Set the section level of the game
 * The setSectionLevel() function sets the current 'section' of the game.
 * This is the number that is displayed under the current level.
 *
 * @param sectionLevel Integer representing the section level
 * @returns nothing
 * @usage setSectionLevel(sectionLevel)
 */
static int setSectionLevel(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->sectionLevel = (int)lua_tonumber(lua, 1);
    return 0;
}

/*
 * lua function
 * @brief Get the section level of the game
 * This function returns the current section level of the game.
 *
 * @returns Integer representing the section level
 * @usage getSectionLevel()
 */
static int getSectionLevel(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    lua_pushnumber(lua, mode->sectionLevel);
    return 1;
}

/*
 * lua function
 * @brief Set the game level
 * The setLevel() function sets the current game level.
 *
 * @param level Integer representing the game level
 * @returns nothing
 * @usage setLevel(level)
 */
static int setLevel(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->level = (int)lua_tonumber(lua, 1);
    return 0;
}

/*
 * lua function
 * @brief Get the game level
 * This function returns the current game level.
 *
 * @returns Integer representing the game level
 * @usage getLevel()
 */
static int getLevel(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    lua_pushnumber(lua, mode->level);
    return 1;
}

/*
 * lua function
 * @brief Set the game score
 * The setScore() function sets the current score of the game.
 *
 * @param score Integer representing the game score
 * @returns nothing
 * @usage setScore(score)
 */
static int setScore(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.score = (int)lua_tonumber(lua, 1);
    return 0;
}

/*
 * lua function
 * @brief Add to the game score
 * The addScore() function increments the current score of the game by a given amount.
 *
 * @param amount Integer to be added to the current score
 * @returns nothing
 * @usage addScore(amount)
 */
static int addScore(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.score += (int)lua_tonumber(lua, 1);
    return 0;
}

/*
 * lua function
 * @brief Get the game score
 * This function returns the current score of the game.
 *
 * @returns Integer representing the current game score
 * @usage getScore()
 */
static int getScore(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    lua_pushnumber(lua, mode->rule.score);
    return 1;
}

/*
 * lua function
 * @brief Set the soft drop frame count
 * The setSoftFrames() function sets the amount of frames a Tetrimino is held down for during a soft drop.
 *
 * @param frames Integer representing the number of frames
 * @returns nothing
 * @usage setSoftFrames(frames)
 */
static int setSoftFrames(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->rule.linesSoftDropped = (int)lua_tonumber(lua, 1);
    return 0;
}

/*
 * lua function
 * @brief Get the soft drop frame count
 * This function returns the amount of frames a Tetrimino is held down for during a soft drop.
 *
 * @returns Integer representing the number of soft drop frames
 * @usage getSoftFrames()
 */
static int getSoftFrames(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    lua_pushnumber(lua, mode->rule.linesSoftDropped);
    return 1;
}

/*
 * lua function
 * @brief Set whether to display the Tetriminos as bone blocks
 * Bone blocks are a visual style that display Tetriminos as monochrome brackets [ ].
 * The setBoneBlocks() function enables or disables this feature.
 *
 * @param enableBoneBlocks Boolean indicating whether to enable Bone Blocks or not
 * @returns nothing
 * @usage setBoneBlocks(enableBoneBlocks)
 */
static int setBoneBlocks(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    mode->boneBlocks = (bool) lua_toboolean(lua, 1);
    return 1;
}

/*
 * lua function
 * @brief Get the elapsed game time
 * This function returns the total amount of time passed since the game started.
 *
 * @returns Float representing the elapsed time in seconds
 * @usage getElapsedTime()
 */
static int getElapsedTime(lua_State* lua)
{
    lua_getglobal(lua, "mode");
    GameMode* mode = (GameMode*) lua_touserdata(lua, -1);

    lua_pushnumber(lua, GetGameTimerElapsed(mode->gameTimer));
    return 1;
}
