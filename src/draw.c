#include <raylib.h>
#include "draw.h"
#include "engine.h"
#include "main.h"
#include "piece.h" // for pieces[7][4][4][4] global matrix containing every piece/rotation

#define BOARD_BG (Color){ 0, 0, 0, 240 }
#define SHADOW (Color){ 231, 231, 231, 255 }
#define GHOST (Color){ 255, 255, 255, 100 } 


void drawBorder(Vector2 playFieldPos, Texture2D borderTileset, int cellSize, Color drawColor) {
    /*
    Vector2 leftStartPos = {playFieldPos.x, playFieldPos.y};
    Vector2 leftEndPos = {playFieldPos.x, playFieldPos.y + (matrixHeight * 32)};
    
    Vector2 rightStartPos = {playFieldPos.x + (matrixWidth * 32), playFieldPos.y};
    Vector2 rightEndPos = {playFieldPos.x + (matrixWidth * 32), playFieldPos.y + (matrixHeight * 32)};

    DrawLineEx(leftStartPos, leftEndPos, 5, BLACK);
    DrawLineEx(rightStartPos, rightEndPos, 5, BLACK);*/

    Rectangle topLeft = {0, 0, cellSize, cellSize};
    Rectangle topRight = {2 * cellSize, 0, cellSize, cellSize};

    Rectangle bottomLeft = {0, 2 * cellSize, cellSize, cellSize};
    Rectangle bottomRight = {2 * cellSize, 2 * cellSize, cellSize, cellSize};

    Rectangle sideLeft = {0, cellSize, cellSize, cellSize};
    Rectangle sideRight = {2 * cellSize, cellSize, cellSize, cellSize};

    Rectangle top = {cellSize, 0, cellSize, cellSize};
    Rectangle bottom = {cellSize, 2 * cellSize, cellSize, cellSize};

    for (int y = 2; y < matrixHeight; y++) {
        for (int x = 0; x < matrixWidth + 2; x++) {
            if (y == 2) {
                if (x == 0)
                    DrawTextureRec(borderTileset, topLeft, (Vector2){playFieldPos.x - cellSize, playFieldPos.y + (cellSize)}, drawColor);
                else if (x == matrixWidth + 1)
                    DrawTextureRec(borderTileset, topRight, (Vector2){playFieldPos.x + (x * cellSize) - cellSize, playFieldPos.y + (cellSize)}, drawColor);
                else
                    DrawTextureRec(borderTileset, top, (Vector2){playFieldPos.x + (x * cellSize) - cellSize, playFieldPos.y + (cellSize)}, drawColor);
            }
            else if (y == matrixHeight - 1) {
                if (x == 0)
                    DrawTextureRec(borderTileset, bottomLeft, (Vector2){playFieldPos.x - cellSize, playFieldPos.y + cellSize + (cellSize * y)}, drawColor);
                else if (x == matrixWidth + 1)
                    DrawTextureRec(borderTileset, bottomRight, (Vector2){playFieldPos.x + (x * cellSize) - cellSize, playFieldPos.y + cellSize + (cellSize * y)}, drawColor);
                else
                    DrawTextureRec(borderTileset, bottom, (Vector2){playFieldPos.x + (x * cellSize) - cellSize, playFieldPos.y + cellSize + (cellSize * y)}, drawColor);
            }
        }
        if (y != 1) {
            DrawTextureRec(borderTileset, sideLeft, (Vector2){playFieldPos.x - cellSize, playFieldPos.y + (cellSize * y)}, drawColor);
            DrawTextureRec(borderTileset, sideRight, (Vector2){playFieldPos.x + (cellSize * matrixWidth), playFieldPos.y + (cellSize * y)}, drawColor);
        }
    }
}

void drawPiecePreview(Piece activePiece, Vector2 playFieldPos, int cellSize) {
    for (int i = 0; i < 3; i++) {
        for (int y = 0; y < 4; y++) {
            for (int x = 0; x < 4; x++) {
                int idx = upcomingPieces[i];
                if (i == 0) {
                    if (pieces[idx][0][y][x] != 0) {
                        Rectangle frameRec = {pieces[idx][0][y][x]*cellSize, 0, cellSize, cellSize};
                        Vector2 pos = {playFieldPos.x + (matrixWidth * 32 / 2) + (x*cellSize) - (cellSize * 2), playFieldPos.y + (y*cellSize) - (cellSize * 2)};
                        DrawTextureRec(activePiece.tileset, frameRec, pos, WHITE); 
                    }   
                }
                else {
                    if (pieces[idx][0][y][x] != 0) {
                        Rectangle frameRec = {pieces[idx][0][y][x]*cellSize, 0, cellSize, cellSize};
                        Rectangle pos = 
                        {
                            playFieldPos.x + (matrixWidth * 32 / 2) + ((x*cellSize)/2) + (cellSize * 2)+(cellSize/2),
                            playFieldPos.y + ((y*cellSize)/2) - (cellSize/2),
                            cellSize / 2,
                            cellSize / 2
                        };
                        if (i == 1) {
                            DrawTexturePro(activePiece.tileset, frameRec, pos, (Vector2){0,0}, 0, WHITE);
                        }
                        else DrawTexturePro(activePiece.tileset, frameRec, (Rectangle){pos.x + (i * cellSize) + (cellSize/2), pos.y, pos.width, pos.height}, (Vector2){0,0}, 0, WHITE);
                    }   
                }
            }   
        }   
    }   
    DrawText("NEXT", playFieldPos.x + (matrixWidth * 32 / 2) - cellSize, playFieldPos.y - (cellSize * 2), 20, GREEN);

}

void drawHeldPiece(Piece activePiece, Vector2 playFieldPos, int cellSize) {
    if (heldPiece != -1) {
        for (int y = 0; y < 4; y++) {
            for (int x = 0; x < 4; x++) {

                if (pieces[heldPiece][0][y][x] != 0) {
                    Rectangle frameRec = {pieces[heldPiece][0][y][x]*cellSize, 0, cellSize, cellSize};
                    Rectangle pos = 
                    {
                        playFieldPos.x + (x*cellSize/2),
                        playFieldPos.y + ((y*cellSize)/2) - (cellSize/2),
                        cellSize / 2,
                        cellSize / 2
                    };
                    DrawTexturePro(activePiece.tileset, frameRec, pos, (Vector2){0,0}, 0, WHITE);
                }
            }
        }
    }
    DrawText("HELD", playFieldPos.x, playFieldPos.y - ((cellSize / 2) * 3), 20, YELLOW);
}

void drawActivePiece(Piece activePiece, Vector2 playFieldPos, int cellSize) {
    if (canDrawPiece()) {
        for (int y = 0; y < 4; y++) {
            for (int x = 0; x < 4; x++) {
               if (pieces[activePiece.pieceIndex][activePiece.rotIndex][y][x] != 0) {
                   Rectangle frameRec = {pieces[activePiece.pieceIndex][activePiece.rotIndex][y][x]*cellSize, 0, cellSize, cellSize};
                   Vector2 pos = {((activePiece.position.x+x)*cellSize + playFieldPos.x), 
                       ((activePiece.position.y+y)*cellSize) + playFieldPos.y};

                   DrawTextureRec(activePiece.tileset, frameRec, pos, WHITE); 
               } 
            }
        }
    }

}

void drawGhostPiece(Piece* activePiece, Block** playField, Vector2 playFieldPos, int cellSize) {
    if (canDrawPiece()) {
        for (int y = 0; y < 4; y++) {
            for (int x = 0; x < 4; x++) {
               if (pieces[activePiece->pieceIndex][activePiece->rotIndex][y][x] != 0) {

                   Rectangle frameRec = {pieces[activePiece->pieceIndex][activePiece->rotIndex][y][x]*cellSize, 0, cellSize, cellSize};
                   Vector2 ghostPos = getFinalPos(activePiece, playField);

                   Vector2 pos = {((ghostPos.x+x)*cellSize + playFieldPos.x), 
                                 ((ghostPos.y+y)*cellSize) + playFieldPos.y};
                   
                   DrawTextureRec(activePiece->tileset, frameRec, pos, GHOST); 
               }
            }
        }
    }
}

void drawPlayField(Block** playField, Vector2 playFieldPos, int cellSize, bool isInvisible) {
    DrawRectangle(playFieldPos.x, playFieldPos.y + (2 * cellSize), matrixWidth * cellSize, (matrixHeight -2) * cellSize, BOARD_BG);
    if (!isInvisible) {
        for (int y = 0; y < matrixHeight; y++) {
            for (int x = 0; x < matrixWidth; x++) {
                Rectangle frameRec = {playField[y][x].type*cellSize, 0, cellSize, cellSize};
                Vector2 position = {playFieldPos.x + (x*cellSize), playFieldPos.y + (y*cellSize)};
                Vector2 shadowPos = position;
                shadowPos.x += 6;
                shadowPos.y += 6;
                DrawTextureRec(playField[y][x].tileset, frameRec, shadowPos, SHADOW); // 3d effect
                DrawTextureRec(playField[y][x].tileset, frameRec, position, WHITE);
            }
        }
    }
}

void drawStackOutline(Block** playField, Vector2 playFieldPos, int cellSize, bool isInvisible) {
    if (!isInvisible) {
        for (int y = 0; y < matrixHeight; y++) {
            for (int x = 0; x < matrixWidth; x++) {
                if (playField[y][x].type != 0) {
                    int top = y - 1;
                    int bottom = y + 1;
                    int left = x - 1;
                    int right = x + 1;

                    if (playField[top][x].type == 0) {
                        Vector2 startPos = {playFieldPos.x + (x * cellSize), playFieldPos.y + (y * cellSize) - 1};
                        Vector2 endPos = {playFieldPos.x + (x * cellSize) + cellSize, playFieldPos.y + (y * cellSize) - 1};
                        DrawLineEx(startPos, endPos, 2.f, WHITE);
                    }
                    
                    if (bottom <= matrixHeight - 1 && playField[bottom][x].type == 0) {
                        Vector2 startPos = {playFieldPos.x + (x * cellSize), playFieldPos.y + (bottom * cellSize) + 1};
                        Vector2 endPos = {playFieldPos.x + (x * cellSize) + cellSize, playFieldPos.y + (bottom * cellSize) + 1};
                        DrawLineEx(startPos, endPos, 2.f, WHITE);
                    }

                    if (left >= 0 && playField[y][left].type == 0) {
                        Vector2 startPos = {playFieldPos.x + (x * cellSize) - 1, playFieldPos.y + (y * cellSize)};
                        Vector2 endPos = {playFieldPos.x + (x * cellSize) - 1, playFieldPos.y + (y * cellSize) + cellSize};
                        DrawLineEx(startPos, endPos, 2.f, WHITE);
                    }
                    
                    if (right <= matrixWidth - 1 && playField[y][right].type == 0) {
                        Vector2 startPos = {playFieldPos.x + (x * cellSize) + cellSize + 1, playFieldPos.y + (y * cellSize)};
                        Vector2 endPos = {playFieldPos.x + (x * cellSize) + cellSize + 1, playFieldPos.y + (y * cellSize) + cellSize};
                        DrawLineEx(startPos, endPos, 2.f, WHITE);
                    }
                }
            }
        }
    }
}

void drawMenu(int gameType, Vector2 playFieldPos) {
    Color masterColor = WHITE;
    Color deathColor = WHITE;
    Color easyColor = WHITE;
    Color invisColor = WHITE;
    Color color20g = WHITE;

    switch (gameType) {
        case 0:
            masterColor = BLUE;
            break;
        case 1:
            deathColor = RED;
            break;
        case 2:
            easyColor = GREEN;
            break;
        case 3:
            invisColor = YELLOW;
            break;
        case 4:
            color20g = PURPLE;
            break;
    }

    DrawText("Tuxmino Master", playFieldPos.x, 120 + playFieldPos.y, 40, masterColor);
    DrawText("Death", playFieldPos.x, 160 + playFieldPos.y, 40, deathColor);
    DrawText("Easy", playFieldPos.x, 200 + playFieldPos.y, 40, easyColor);
    DrawText("Invisible", playFieldPos.x, 240 + playFieldPos.y, 40, invisColor);
    DrawText("20G practice", playFieldPos.x, 280 + playFieldPos.y, 40, color20g);
}

void drawPauseMenu(int idxOption, Vector2 playFieldPos) {
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

    DrawText("Resume", playFieldPos.x, 120 + playFieldPos.y, 40, resumeColor);
    DrawText("Main Menu", playFieldPos.x, 160 + playFieldPos.y, 40, mainMenuColor);
    DrawText("Quit", playFieldPos.x, 200 + playFieldPos.y, 40, quitColor);
}

void drawGameOverMenu(Vector2 playFieldPos, int cellSize) {
    DrawRectangle(playFieldPos.x, playFieldPos.y + (2 * cellSize), matrixWidth * cellSize, (matrixHeight -2) * cellSize, (Color){0,0,0,220});
    DrawText("GAME OVER", playFieldPos.x, playFieldPos.y + ((cellSize * matrixHeight) / 2), 52, WHITE);
    DrawText("Press Enter to Restart", playFieldPos.x + 3, playFieldPos.y + ((cellSize * matrixHeight) / 2) + 100, 26, RED);
}
