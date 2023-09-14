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

    const CreditEntry creditsList[CREDITS_LENGTH] = {
        { "Lead Programmer", GREEN },
        { "Mason Armand", WHITE },
        { "Github Contributors", GREEN },
        { "Mason Armand", WHITE },
        { "Dawnvoid", WHITE },
        { "Art Made By", GREEN },
        { "errormine", WHITE },
        { "Mason Armand", WHITE },
        { "Creators Website", GREEN },
        { "masonarmand.com", WHITE },
        { "tuxmino.org", GREEN },

    };

    RenderTexture2D tex = LoadRenderTexture(playField.width * playField.cellSize, height);
    RenderTexture2D texFinal = LoadRenderTexture(playField.width * playField.cellSize, height);
    Texture2D endTexture = LoadTexture("res/decor/credits-end.png");


    BeginTextureMode(tex);
    for (int i = CREDITS_LENGTH - 1; i >= 0; i--) {
        DrawCenteredText(creditsList[i].str, playField.width * playField.cellSize, 0, startPosY + (spacing * i), creditsList[i].color);
    }
    DrawTexture(endTexture, 0, tex.texture.height - 640, WHITE);
    EndTextureMode();

    BeginTextureMode(texFinal);
        DrawTextureRec(tex.texture, (Rectangle){0,0,playField.width * playField.cellSize, height}, (Vector2){0,0}, WHITE);
    EndTextureMode();

    return texFinal;
}
