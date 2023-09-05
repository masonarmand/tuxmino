#include "tuxmino.h"


#define CREDITS_LENGTH 11


void DrawCenteredText(const char* text, int width, float x, float y, Color color);

void DrawCenteredText(const char* text, int width, float x, float y, Color color) {
    int center = (width / 2) - (MeasureText(text, 30) / 2);
    DrawText(text, x + center, y, 30, color);
}

RenderTexture2D renderCreditsTexture(PlayField playField) {
    float startPosY = (playField.cellSize * playField.height);
    float spacing = 50;
    float height = (startPosY * 2) + (CREDITS_LENGTH * spacing);

    const char* creditsList[CREDITS_LENGTH][2] = {
        {"Lead Programmer", "g"},
        {"Mason Armand", "w"},
        {"Github Contributors", "g"},
        {"Mason Armand", "w"},
        {"Dawnvoid", "w"},
        {"Art Made By", "g"},
        {"errormine", "w"},
        {"Mason Armand", "w"},
        {"Creators Website", "g"},
        {"masonarmand.com", "w"},
        {"tuxmino.org", "w"},

    };

    RenderTexture2D tex = LoadRenderTexture(playField.width * playField.cellSize, height);
    RenderTexture2D texFinal = LoadRenderTexture(playField.width * playField.cellSize, height);
    Texture2D endTexture = LoadTexture("res/decor/credits-end.png");


    BeginTextureMode(tex);
    for (int i = CREDITS_LENGTH - 1; i >= 0; i--) {
        Color color = WHITE;
        if (strcmp(creditsList[i][1], "g") == 0)
            color = GREEN;
        DrawCenteredText(creditsList[i][0], playField.width * playField.cellSize, 0, startPosY + (spacing * i), color);
    }
    DrawTexture(endTexture, 0, tex.texture.height - 640, WHITE);
    EndTextureMode();

    BeginTextureMode(texFinal);
        DrawTextureRec(tex.texture, (Rectangle){0,0,playField.width * playField.cellSize, height}, (Vector2){0,0}, WHITE);
    EndTextureMode();

    return texFinal;
}
