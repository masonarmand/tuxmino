/*
 * File: draw.c
 * ------------
 * Functions/abstractions for drawing
 * various game elements to the screen
 *
 * Original Author: Mason Armand
 * Contributors: Dawnvoid
 * Date Created: Sep 14, 2022
 * Last Modified: Mar 4, 2023
 */

#include "tuxmino.h"
#include <stdio.h>

#define BOARD_BG (Color){ 0, 0, 0, 220 }
#define SHADOW (Color){ 231, 231, 231, 255 }
#define GHOST_COLOR (Color){ 130, 130, 130, 255 }
#define GHOST3D_COLOR (Color){ 50, 50, 50, 255 }
#define FRAME_SIZE 21

static Timer gameOverAnim;

/* Internal functions */
static void drawBlock(int type, Texture2D texture, Vector2 position, Color color, float cellSize, float drawSize);
static void draw3dEffect(int type, Texture2D texture, Vector2 position, Color color, float cellSize, float drawSize);
static void drawPiece(int pieceId, int rotation, Texture2D texture, Vector2 position, Color color, float cellSize, float drawSize, bool effect3D);


static void drawBlock(int type, Texture2D texture, Vector2 position, Color color, float cellSize, float drawSize)
{
    Vector2 origin = { 0, 0 };

    Rectangle frameRec = (Rectangle){type * cellSize, 0, cellSize, cellSize};
    Rectangle destRec = {
        position.x,
        position.y,
        drawSize,
        drawSize
    };
    DrawTexturePro(texture, frameRec, destRec, origin, 0.0f, color);
}


/*
 * Draw pseudo 3d effect on blocks
 */
static void draw3dEffect(int type, Texture2D texture, Vector2 position, Color color, float cellSize, float drawSize)
{
    Vector2 origin = { 0, 0 };

    Rectangle frameRec = (Rectangle){type * cellSize, 0, cellSize, cellSize};
    Rectangle destRec = {
        position.x + drawSize,
        position.y,
        drawSize / 3.0f,
        drawSize
    };

    DrawTexturePro(texture, frameRec, destRec, origin, 0, color);
}


static void drawPiece(int pieceId, int rotation, Texture2D texture, Vector2 position, Color color, float cellSize, float drawSize, bool effect3D)
{
    for (unsigned int y = 0; y < 4; y++) {
        for (unsigned int x = 0; x < 4; x++) {
            int type = pieces[pieceId][rotation][y][x];

            if (type == 0) {
                continue;
            }

            Vector2 pos = {
                position.x + (x * drawSize),
                position.y + (y * drawSize)
            };

            if (effect3D) {
                Color color3d = { color.r - 50, color.g - 50, color.b - 50, color.a - 50 };
                draw3dEffect(type, texture, pos, color3d, cellSize, drawSize);
            }

            drawBlock(type, texture, pos, color, cellSize, drawSize);
        }
    }
}



/*
 * Draw joystick visual of what keys are currently being pressed
 */
void drawKeyPresses(int screenWidth, int ScreenHeight)
{
    Vector2 joystickPos = {screenWidth - 90, ScreenHeight - 36};
    Vector2 balltopPos = joystickPos;

    Vector2 leftRotatePos = {joystickPos.x + 32, joystickPos.y - 4};
    Vector2 rightRotatePos = {leftRotatePos.x + 20, leftRotatePos.y};
    Vector2 leftRotatePosAlt = {rightRotatePos.x + 20, rightRotatePos.y};
    Vector2 holdPos = {leftRotatePos.x - 4, leftRotatePos.y + 20};

    Color buttonDefaultColor = (Color){60, 60, 60, 255};

    Color leftRotateColor = buttonDefaultColor;
    Color rightRotateColor = buttonDefaultColor;
    Color leftRotateAltColor = buttonDefaultColor;
    Color holdColor = buttonDefaultColor;
    Color joyStickColor = buttonDefaultColor;

    if (IsKeyDown(config.KeyBinds.up)) {
        balltopPos.y = joystickPos.y - 20;
        joyStickColor = MAROON;
    }
    else if (IsKeyDown(config.KeyBinds.down)) {
        balltopPos.y = joystickPos.y + 20;
        joyStickColor = MAROON;
    }

    if (IsKeyDown(config.KeyBinds.left)) {
        balltopPos.x = joystickPos.x - 20;
        joyStickColor = MAROON;
    }
    else if (IsKeyDown(config.KeyBinds.right)) {
        balltopPos.x = joystickPos.x + 20;
        joyStickColor = MAROON;
    }

    if (IsKeyDown(config.KeyBinds.ccw))
        leftRotateColor = MAROON;

    if (IsKeyDown(config.KeyBinds.ccwAlt))
        leftRotateAltColor = MAROON;

    if (IsKeyDown(config.KeyBinds.cw))
        rightRotateColor = MAROON;

    if (IsKeyDown(config.KeyBinds.swapHold))
        holdColor = MAROON;

    DrawCircle(screenWidth - 20, ScreenHeight, 100, (Color){0, 0, 10, 255});
    DrawCircle(leftRotatePos.x, leftRotatePos.y, 8, leftRotateColor);
    DrawCircle(rightRotatePos.x, rightRotatePos.y, 8, rightRotateColor);
    DrawCircle(leftRotatePosAlt.x, leftRotatePosAlt.y, 8, leftRotateAltColor);
    DrawCircle(holdPos.x, holdPos.y, 8, holdColor);

    DrawLineEx(joystickPos, balltopPos, 4, WHITE);
    DrawCircle(balltopPos.x, balltopPos.y, 10, joyStickColor);
}


/*
 * Draw title screen options
 */
void drawTitleMenu(Vector2 pos, int idxOption)
{
    const char* titleText = "Tuxmino";
    const char* modesText = "Game Modes";
    const char* optionsText = "Options";
    const char* quitText = "Quit";

    int fontSize = 40;

    float spacing = 64.0f;

    Color modesColor = WHITE;
    Color optionsColor = WHITE;
    Color quitColor = WHITE;

    switch (idxOption) {
        case 0:
            modesColor = GREEN;
            break;
        case 1:
            optionsColor = BLUE;
            break;
        case 2:
            quitColor = RED;
            break;
    }

    /* x position centering for menu options */
    float pos0 = pos.x - (MeasureText(titleText, fontSize) / 2.0f);
    float pos1 = pos.x - (MeasureText(modesText, fontSize) / 2.0f);
    float pos2 = pos.x - (MeasureText(optionsText, fontSize) / 2.0f);
    float pos3 = pos.x - (MeasureText(quitText, fontSize) / 2.0f);

    DrawText(titleText, pos0, pos.y, 40, RED);
    DrawText(modesText, pos1, pos.y + spacing, fontSize, modesColor);
    DrawText(optionsText, pos2, pos.y + (spacing * 2), fontSize, optionsColor);
    DrawText(quitText, pos3, pos.y + (spacing * 3), fontSize, quitColor);
}


/*
 * Draws the frame of the playfield
 */
void drawBorder(Texture2D borderTileset, PlayField playField, GameMode mode)
{
    float cellSize = playField.cellSize;

    Rectangle topLeft = {0, 0, cellSize, cellSize};
    Rectangle topRight = {2 * cellSize, 0, cellSize, cellSize};

    Rectangle bottomLeft = {0, 2 * cellSize, cellSize, cellSize};
    Rectangle bottomRight = {2 * cellSize, 2 * cellSize, cellSize, cellSize};

    Rectangle sideLeft = {0, cellSize, cellSize, cellSize};
    Rectangle sideRight = {2 * cellSize, cellSize, cellSize, cellSize};

    Rectangle top = {cellSize, 0, cellSize, cellSize};
    Rectangle bottom = {cellSize, 2 * cellSize, cellSize, cellSize};

    Color color = mode.frameColor;

    /*
     * This loop starts at index 2 because the playfield matrix is actually
     * 2 blocks higher than it appears to be (for piece rotation at the top of
     * the playing field)
     */
    for (unsigned int y = 2; y < playField.height; y++) {
        for (unsigned int x = 0; x < playField.width + 2; x++) {
            Vector2 topPos = {
                playField.pos.x + (x * cellSize) - cellSize,
                playField.pos.y + cellSize
            };

            Vector2 bottomPos = {
                playField.pos.x + (x * cellSize) - cellSize,
                playField.pos.y + cellSize + (cellSize * y)
            };

            /* top left border */
            if (x == 0 && y == 2) {
                DrawTextureRec(borderTileset, topLeft, topPos, color);
                continue;
            }

            /* top right border */
            else if (x == playField.width + 1 && y == 2) {
                DrawTextureRec(borderTileset, topRight, topPos, color);
                continue;
            }

            /* top border */
            else if (y == 2) {
                DrawTextureRec(borderTileset, top, topPos, color);
                continue;
            }

            /* bottom left border */
            if (x == 0 && y == playField.height - 1) {
                DrawTextureRec(borderTileset, bottomLeft, bottomPos, color);
                continue;
            }

            /* bottom right border */
            else if (x == playField.width + 1 && y == playField.height - 1) {
                DrawTextureRec(borderTileset, bottomRight, bottomPos, color);
                continue;
            }

            /* bottom border */
            else if (y == playField.height - 1) {
                DrawTextureRec(borderTileset, bottom, bottomPos, color);
                continue;
            }
        }

        /* border sides */
        Vector2 sideLeftPos = {
            playField.pos.x - cellSize,
            playField.pos.y + (cellSize * y)
        };

        Vector2 sideRightPos = {
            playField.pos.x + (cellSize * playField.width),
            playField.pos.y + (cellSize * y)
        };

        DrawTextureRec(borderTileset, sideLeft, sideLeftPos, color);
        DrawTextureRec(borderTileset, sideRight, sideRightPos, color);
    }
}


/*
 * Draw grade directly using specified tileset and position
 */
void drawGradeEx(Texture2D gradeTexture, int grade, Vector2 gradePos)
{
    int tilesetX = grade;
    int tilesetY = 0;

    if (grade > 17) {
        tilesetX = 0;
        tilesetY = 2;
    }
    else if (grade > 8) {
        tilesetX = grade - 9;
        tilesetY = 1;
    }

    DrawTextureRec(
        gradeTexture,
        (Rectangle) { tilesetX * 96, tilesetY * 96, 96, 96},
        gradePos,
        WHITE
    );
}


/*
 * Built in abstraction for drawGradeEx, position is preset. Position and style
 * is changed depending on SpeedSettings.gradeType
 */
void drawGrade(Texture2D gradeTexture, Texture2D altGradeTexture, SpeedSettings rule, PlayField playField)
{
    if (!rule.displayGrade) {
        return;
    }

    Vector2 pos;
    Texture2D tex;

    float cellSize = playField.cellSize;

    if (rule.gradeType == 0) {
        tex = gradeTexture;
        pos.x = playField.pos.x - 138;
    }
    else if (rule.gradeType == 1) {
        tex = altGradeTexture;
        pos.x = playField.pos.x + (cellSize * playField.width) + FRAME_SIZE + 10;
    }

    pos.y = playField.pos.y + (FRAME_SIZE * 2);
    drawGradeEx(tex, rule.grade, pos);
}


/*
 * Built in function for drawing score with preset position
 */
void drawScore(SpeedSettings rule, PlayField playField)
{
    char* scoreText = (char*) TextFormat("%d", rule.score);
    float cellSize = playField.cellSize;
    int scoreX = MeasureText(scoreText, 30);
    int posX = playField.pos.x - 120;
    int posY = playField.pos.y + 385;
    int numPosX = playField.pos.x - scoreX - cellSize - 10;
    int numPosY = posY + 20;

    if (rule.gradeType == 1) {
        posX = playField.pos.x + (cellSize * playField.width) + FRAME_SIZE + 10;
        numPosX = posX;
    }

    DrawText("POINTS", posX, posY, 20, WHITE);
    DrawText(scoreText, numPosX, numPosY, 30, WHITE);
}


/*
 * Built in function for drawing level counter with preset position
 */
void drawLevelCount(SpeedSettings rule, PlayField playField, GameMode mode)
{

    const char* levelText = TextFormat("%03d", mode.level);
    const char* sectionLevelText = TextFormat("%03d", mode.sectionLevel);

    float cellSize = playField.cellSize;
    float playFieldHeight = playField.pos.y + playField.height * cellSize;
    float playFieldWidth = playField.pos.x + playField.width * cellSize;

    int width = MeasureText(sectionLevelText, 40);
    float posX = playField.pos.x - FRAME_SIZE - 20.0f - width;
    float currentLevelPosY = playFieldHeight - 60.0f;
    float maxLevelPosY = playFieldHeight - 20.0f;
    float lineY = playFieldHeight - 23.0f;

    if (rule.gradeType == 1)
        posX = playFieldWidth + FRAME_SIZE + 10;

    Vector2 lineStartPos = { posX, lineY };
    Vector2 lineEndPos = { posX + width, lineY };

    DrawText(levelText, posX, currentLevelPosY, 40, WHITE);
    DrawLineEx(lineStartPos, lineEndPos, 1.5f, WHITE);
    DrawText(sectionLevelText, posX, maxLevelPosY, 40, WHITE);
}


/*
 * Draw scrolling creditsTexture, speed is dependant on how long the credit roll
 * is.
 */
void drawCredits(RenderTexture2D creditsTexture, Timer creditRollTimer, PlayField playField)
{
    float travel = (creditsTexture.texture.height - 640) / creditRollTimer.lifeTime;
    float yPos = travel * GetElapsed(creditRollTimer);
    float cellSize = playField.cellSize;

    if (TimerDone(creditRollTimer))
        yPos = creditsTexture.texture.height - 640;

    Rectangle sourceRect = { 0, yPos, 320, 640 };
    Vector2 pos = { playField.pos.x, playField.pos.y + (cellSize * 2) };

    DrawTextureRec(creditsTexture.texture, sourceRect, pos, WHITE);
}


/*
 * Built in function for drawing the Master1 style next grade. This function
 * could potentially be removed and done soley through lua.
 */
void drawNextGrade(SpeedSettings rule, PlayField playField)
{
    if (!rule.displayGrade || rule.gradeType != 0) {
        return;
    }

    float cellSize = playField.cellSize;
    float pos = playField.pos.y + 145.0f;

    const char* nextScoreText = TextFormat("%d", rule.nextScore);

    int width = MeasureText(nextScoreText, 30);
    float nextScoreX = playField.pos.x - width - cellSize - 10.0f;

    DrawText("NEXT GRADE AT", playField.pos.x - 210.0f, pos, 20, WHITE);
    DrawText(nextScoreText, nextScoreX, pos + 20.0f, 30, WHITE);
    DrawText("POINTS", playField.pos.x - 120.0f, pos + 50.0f, 20, WHITE);
}


void drawLineClearEffect(Texture2D explosionTileset, PlayField playField, int currentFrame, int currentLine, int row)
{
    int size = 192;
    int center = size / 2;
    /*
     * In the animation image, the particle effect
     * is on the top 1/4th of the animation cell
     */
    int yOrigin = size / 4;

    Rectangle frameRec = { size * currentFrame, size * currentLine, size, size };
    Color explosionColor;

    float scale = 1.3f;
    float cellSize = playField.cellSize;

    for (unsigned int i = 0; i < playField.width; i++) {
        float blockPosX = playField.pos.x + (i * cellSize);
        float blockPosY = playField.pos.y + (row * cellSize);

        int randRot = GetRandomValue(0, 10);

        Rectangle dest = {blockPosX, blockPosY, size * scale, size * scale};
        Vector2 origin = { center * scale, (yOrigin * scale) };

        switch(playField.copy[row][i].type) {
            case 1: explosionColor = BLUE; break;
            case 2: explosionColor = PURPLE; break;
            case 3: explosionColor = RED; break;
            case 4: explosionColor = GREEN; break;
            case 5: explosionColor = YELLOW; break;
            case 6: explosionColor = SKYBLUE; break;
            case 7: explosionColor = ORANGE; break;
        }

        DrawTexturePro(explosionTileset, frameRec, dest, origin, randRot, explosionColor);
    }
}


void drawPiecePreview(Piece activePiece, SpeedSettings rule, PlayField playField, Texture2D nextBg)
{
    float cellSize = playField.cellSize;

    if (rule.drawNextPieceBG) {
        float yOffset = (cellSize / 4);
        float width = (playField.width * cellSize) + (FRAME_SIZE * 2);
        float height = (cellSize * 2) + yOffset;

        Rectangle sourceRec = { 0, 0, nextBg.width, nextBg.height };

        Rectangle destRec = {
            playField.pos.x - FRAME_SIZE,
            playField.pos.y - FRAME_SIZE - yOffset,
            width,
            height
        };

        DrawTexturePro(nextBg, sourceRec, destRec, (Vector2){0, 0}, 0, WHITE);
    }

    for (unsigned int i = 0; i < rule.piecePreviewCount; i++) {

        int pieceId = upcomingPieces[i];

        float drawSize = cellSize;
        float xOffset = drawSize * 2;
        float yOffset = drawSize / 4;

        Vector2 position = {
            playField.pos.x + (playField.width * cellSize / 2) - xOffset,
            playField.pos.y - cellSize - FRAME_SIZE - yOffset
        };

        if (i != 0) {
            drawSize = cellSize / 2;

            float startX = position.x + (cellSize * 2);
            float indexOffset = i * (drawSize * 5);

            position = (Vector2) {
                startX + indexOffset,
                playField.pos.y - FRAME_SIZE
            };
        }

        drawPiece(pieceId, 0, activePiece.currentTileset, position, WHITE, cellSize, drawSize, rule.effect3D);
    }

    DrawText("NEXT", playField.pos.x - FRAME_SIZE, playField.pos.y + 20, 26, WHITE);
}


void drawHeldPiece(Piece activePiece, SpeedSettings rule, PlayField playField)
{
    if (heldPiece == -1 || !rule.holdEnabled) {
        return;
    }

    float cellSize = playField.cellSize;
    float drawSize = cellSize / 2;

    Vector2 position = {
        (playField.pos.x - drawSize),
        (playField.pos.y - cellSize)
    };

    drawPiece(heldPiece, 0, activePiece.currentTileset, position, WHITE, cellSize, drawSize, rule.effect3D);
}


void drawActivePiece(Piece activePiece, SpeedSettings rule, PlayField playField, Color color)
{
    if (!canDrawPiece()) {
       return;
    }

    float cellSize = playField.cellSize;

    Vector2 position = {
        (activePiece.position.x * cellSize) + playField.pos.x,
        (activePiece.position.y * cellSize) + playField.pos.y
    };

    Texture2D texture = activePiece.currentTileset;

    if (canDrawFlash())
        texture = playField.blankBlockTexture;

    drawPiece(
        activePiece.pieceIndex,
        activePiece.rotIndex,
        texture,
        position,
        color,
        cellSize,
        cellSize,
        rule.effect3D
    );
}


void drawGhostPiece(Piece activePiece, SpeedSettings rule, PlayField playField)
{
    if (!canDrawPiece() || !rule.showGhost) {
        return;
    }

    float cellSize = playField.cellSize;

    Vector2 matrixPos = getFinalPos(&activePiece, playField);

    Vector2 position = {
        matrixPos.x * cellSize + playField.pos.x,
        matrixPos.y * cellSize + playField.pos.y
    };

    Texture2D texture = activePiece.currentTileset;

    if (canDrawFlash())
        texture = playField.blankBlockTexture;

    drawPiece(
        activePiece.pieceIndex,
        activePiece.rotIndex,
        texture,
        position,
        GHOST_COLOR,
        cellSize,
        cellSize,
        rule.effect3D
    );
}


void drawPlayFieldBack(PlayField playField, Color color)
{
    float cellSize = playField.cellSize;

    DrawRectangle(
        playField.pos.x,
        playField.pos.y + (2 * cellSize),
        playField.width * cellSize,
        (playField.height - 2) * cellSize,
        color
    );
}


/*
 * Draw the pseudo 3d effect on entire playField.
 */
void drawPlayField3DEffect(SpeedSettings rule, PlayField playField)
{
    if (rule.invisiblePieces || !rule.effect3D) {
        return;
    }

    float cellSize = playField.cellSize;

    for (unsigned int y = 0; y < playField.height; y++) {
        for (unsigned int x = 0; x < playField.width; x++) {

            int type = playField.matrix[y][x].type;

            float posX = playField.pos.x + (x * cellSize);
            float posY = playField.pos.y + (y * cellSize);

            Vector2 position = { posX, posY };
            Texture2D texture;
            Color drawColor;

            if (type == -1) {
                texture = playField.blankBlockTexture;
                drawColor = (Color){50, 50, 50, 255};
            }
            else {
                texture = playField.matrix[y][x].tileset;
                drawColor = GRAY;
            }

            draw3dEffect(type, texture, position, drawColor, cellSize, cellSize);
        }
    }
}


void drawPlayField(PlayField playField, GameMode mode)
{
    if (mode.rule.invisiblePieces) {
        return;
    }

    float cellSize = playField.cellSize;

    for (unsigned int y = 0; y < playField.height; y++) {
        for (unsigned int x = 0; x < playField.width; x++) {

            int type = playField.matrix[y][x].type;

            float posX = playField.pos.x + (x * cellSize);
            float posY = playField.pos.y + (y * cellSize);

            Vector2 position = {posX, posY};
            Texture2D texture;
            Color drawColor;

            if (type == -1) {
                texture = playField.blankBlockTexture;
                drawColor = GRAY;
            }
            else {
                texture = playField.matrix[y][x].tileset;
                drawColor = mode.pieceTint;
            }

            drawBlock(type, texture, position, drawColor, cellSize, cellSize);
        }
    }
}


void drawStackOutline(PlayField playField, bool isInvisible)
{
    if (isInvisible) {
        return;
    }

    float cellSize = playField.cellSize;

    for (unsigned int y = 0; y < playField.height; y++) {
        for (unsigned int x = 0; x < playField.width; x++) {
            if (playField.matrix[y][x].type == 0) {
                continue;
            }
            unsigned int bottom = y + 1;
            unsigned int right = x + 1;

            unsigned int top = 0;
            unsigned int left = 0;

            if (y != 0)
                top = y - 1;
            if (x != 0)
                left = x - 1;

            float leftX = playField.pos.x + (x * cellSize);
            float rightX = leftX + cellSize;
            float topY = playField.pos.y + (y * cellSize);
            float bottomY = playField.pos.y + (bottom * cellSize);
            float thickness = 1.5f;//1.5f;
            Color lineColor = {180, 180, 180, 255};

            /* top outline */
            if (playField.matrix[top][x].type == 0) {
                Vector2 startPos = { leftX, topY };
                Vector2 endPos = { rightX, topY };
                DrawLineEx(startPos, endPos, thickness, lineColor);
            }

            /* bottom outline */
            if (bottom <= playField.height - 1 && playField.matrix[bottom][x].type == 0) {
                Vector2 startPos = { leftX, bottomY };
                Vector2 endPos = { rightX, bottomY };
                DrawLineEx(startPos, endPos, thickness, lineColor);
            }

            /* left outline */
            if (playField.matrix[y][left].type == 0) {
                Vector2 startPos = { leftX, topY };
                Vector2 endPos = { leftX, bottomY };
                DrawLineEx(startPos, endPos, thickness, lineColor);
            }

            /* right outline */
            if (right <= playField.width - 1 && playField.matrix[y][right].type == 0) {
                Vector2 startPos = { rightX, topY };
                Vector2 endPos = { rightX, bottomY };
                DrawLineEx(startPos, endPos, thickness, lineColor);
            }
        }
    }
}


/*
 * Draw gamemode select menu
 */
void drawMenu(int selectedGameType, GameModeList gameModes, PlayField playField)
{
    int itemCount = gameModes.length - 1;
    int selected = selectedGameType;
    int behind;
    int top;
    int bottom;
    int width = 305;

    float cellSize = playField.cellSize;
    float startPosY = playField.pos.y + ((playField.height * cellSize) / 2.0f);
    float posX = playField.pos.x + ((playField.width * cellSize) / 2.0f) - (width / 2.0f);
    float menuSpacing = 30;

    Color nonSelectedColor = (Color) {50, 50, 50, 255};

    switch(selectedGameType) {
        case 0:
            behind = itemCount - 1;
            top = itemCount;
            break;
        case 1:
            behind = itemCount;
            top = selectedGameType - 1;
            break;
        default:
            behind = selectedGameType - 2;
            top = selectedGameType - 1;
            break;
    }

    if (selectedGameType == itemCount)
        bottom = 0;
    else
        bottom = selectedGameType + 1;

    char* behindName = gameModes.modes[behind].name;
    char* topName = gameModes.modes[top].name;
    char* bottomName = gameModes.modes[bottom].name;
    char* selectedName = gameModes.modes[selected].name;

    switch (itemCount) {
        case 0:
            DrawText(selectedName, posX, startPosY, 50, WHITE);
        break;
        case 1:
            DrawText(bottomName, posX, startPosY + menuSpacing, 50, nonSelectedColor);
            DrawText(selectedName, posX, startPosY, 50, WHITE);
        break;
        default:
            DrawText(behindName, posX, startPosY, 50, nonSelectedColor);
            DrawText(topName, posX, startPosY - menuSpacing, 50, nonSelectedColor);
            DrawText(bottomName, posX, startPosY + menuSpacing, 50, nonSelectedColor);
            DrawText(selectedName, posX, startPosY, 50, WHITE);
        break;
    }
}


void drawGameModeSettings(int level, enum rotationSystems rule, PlayField playField, int idxOption)
{
    Color levelColor = WHITE;
    Color ruleColor = WHITE;

    char ruleText[8];

    int levelWidth = MeasureText("Level ", 40);
    int ruleWidth = MeasureText("Rule  ", 40);

    switch (idxOption) {
        case 0: levelColor = ORANGE; break;
        case 1: if (rule == CLASSIC) ruleColor = RED; else ruleColor = BLUE; break;
    }

    if (rule == CLASSIC)
        sprintf(ruleText, "CLASSIC");
    else
        sprintf(ruleText, "WORLD");

    DrawText("Level ", playField.pos.x, playField.pos.y + 80, 40, WHITE);
    DrawText(TextFormat("%d", level), playField.pos.x + levelWidth, playField.pos.y + 80, 40, levelColor);
    DrawText("Rule  ", playField.pos.x, playField.pos.y + 120, 40, WHITE);
    DrawText(ruleText, playField.pos.x + ruleWidth, playField.pos.y + 120, 40, ruleColor);
}


void drawPauseMenu(int idxOption, PlayField playField)
{
    float cellSize = playField.cellSize;
    float lineLength = playField.width * cellSize;
    float linePosY = playField.pos.y + 118.0f;
    float lineStartX = playField.pos.x;
    float lineEndX = playField.pos.x + lineLength;
    float menuY = playField.pos.y + 120.0f;

    int fontSize = 40;

    Vector2 lineStartPos = { lineStartX, linePosY };
    Vector2 lineEndPos = { lineEndX, linePosY };

    Color resumeColor = WHITE;
    Color mainMenuColor = WHITE;
    Color quitColor = WHITE;

    switch (idxOption) {
        case 0:
            resumeColor = GREEN;
            break;
        case 1:
            mainMenuColor = YELLOW;
            break;
        case 2:
            quitColor = RED;
            break;
    }

    drawPlayFieldBack(playField, (Color){ 0, 0, 0, 220 });
    DrawText("Paused", playField.pos.x, 70.0f + playField.pos.y, 50, WHITE);

    /* draw line under paused text */
    DrawLineEx(lineStartPos, lineEndPos, 1.5f, WHITE);

    /* draw menu options */
    DrawText("Resume", playField.pos.x, menuY, fontSize, resumeColor);
    DrawText("Main Menu", playField.pos.x, menuY + fontSize, fontSize, mainMenuColor);
    DrawText("Quit", playField.pos.x, menuY + (fontSize * 2), fontSize, quitColor);
}


void resetGameOverAnim(void)
{
    resetTimer(&gameOverAnim);
}


void drawGameOverMenu(PlayField playField, SpeedSettings* rule, Texture2D gradeTileset)
{
    double interval = gameOverAnim.lifeTime / playField.height;
    float cellSize = playField.cellSize;
    unsigned int rowCount = GetElapsed(gameOverAnim) / interval;

    rule->invisiblePieces = false;

    if (GetElapsed(gameOverAnim) == 0)
        startTimer(&gameOverAnim, 1);

    if (TimerDone(gameOverAnim))
        rowCount = playField.height;

    unsigned int startPosY = playField.height - 1;
    unsigned int endPosY = playField.height - rowCount - 1;

    for (unsigned int y = startPosY; y > endPosY; y--) {
        for (unsigned int x = 0; x < playField.width; x++) {
            if (playField.matrix[y][x].type != 0) {
                playField.matrix[y][x].type = -1;
            }
        }
    }

    if (TimerDone(gameOverAnim)) {
        const char* score = TextFormat("SCORE: %i", rule->score);

        int gameOverTextWidth = MeasureText("GAME OVER", 48) / 2;
        int gradeTextWidth = MeasureText("GRADE:", 30);
        int center = ((playField.width * cellSize) / 2);

        float gamePosX = playField.pos.x + center - gameOverTextWidth;
        float uiPosY = playField.pos.y + 80;

        drawPlayFieldBack(playField, (Color){ 0, 0, 0, 180 });

        DrawText("GAME OVER", gamePosX + 5, uiPosY + 5, 48, BLACK); /* shadow */
        DrawText("GAME OVER", gamePosX, uiPosY, 48, ORANGE);

        DrawText("GRADE:", playField.pos.x + 5, uiPosY + 80, 30, WHITE);
        DrawText(score, playField.pos.x + 5, uiPosY + 160, 30, WHITE);

        drawGradeEx(
            gradeTileset,
            rule->grade,
            (Vector2){playField.pos.x + gradeTextWidth, playField.pos.y + 125}
        );
    }
}


/*
 * Draws GameTimer in the format Minutes:Seconds:Miliseconds
 */
void drawGameTimer(GameTimer timer, PlayField playField)
{
    double time = (GetGameTimerElapsed(timer));
    float cellSize = playField.cellSize;

    float width = MeasureText("00:00:00", 40);
    int minutes = time / 60;
    int seconds = (int)time % 60;
    int ms = ((int)(time * 1000) % 1000);

    char msText[4];

    sprintf(msText, "%02i", ms);
    msText[2] = ' ';
    msText[3] = ' ';

    DrawText(
        TextFormat("%02i:%02i:%02s", minutes, seconds, msText),
        playField.pos.x + ((cellSize * playField.width) / 2) - (width / 2),
        playField.pos.y + (playField.height*cellSize) + FRAME_SIZE, 40, WHITE
    );
}

