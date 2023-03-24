/*
 * File: config.c
 * --------------
 *
 * Original Author: Mason Armand
 * Last Modified: Feb 14, 2023
 * Last Modified: Mar 21, 2023
 */
#include "tuxmino.h"
#include <string.h>

int handler(void* configuration, const char* section, const char* name, const char* value)
{
    Config* pconfig = (Config*) configuration;

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("Settings", "display-width")) {
        pconfig->settings.displayWidth = atoi(value);
    }
    else if (MATCH("Settings", "display-height")) {
        pconfig->settings.displayHeight = atoi(value);
    }
    else if (MATCH("Settings", "FPS")) {
        pconfig->settings.fps = atoi(value);
    }
    /* Keybinds */
    else if (MATCH("Keybinds", "counter-clockwise")) {
        pconfig->keyBinds.ccw = atoi(value);
    }
    else if (MATCH("Keybinds", "counter-clockwise-alt")) {
        pconfig->keyBinds.ccwAlt = atoi(value);
    }
    else if (MATCH("Keybinds", "clockwise")) {
        pconfig->keyBinds.cw = atoi(value);
    }
    else if (MATCH("Keybinds", "swap-hold")) {
        pconfig->keyBinds.swapHold = atoi(value);
    }
    else if (MATCH("Keybinds", "sonic-drop")) {
        pconfig->keyBinds.sonicDrop = atoi(value);
    }
    else if (MATCH("Keybinds", "move-left")) {
        pconfig->keyBinds.left = atoi(value);
    }
    else if (MATCH("Keybinds", "move-right")) {
        pconfig->keyBinds.right = atoi(value);
    }
    else if (MATCH("Keybinds", "move-down")) {
        pconfig->keyBinds.down = atoi(value);
    }
    else if (MATCH("Keybinds", "move-up")) {
        pconfig->keyBinds.up = atoi(value);
    }
    else if (MATCH("Keybinds", "ui-select")) {
        pconfig->keyBinds.uiSelect = atoi(value);
    }
    else if (MATCH("Keybinds", "restart")) {
        pconfig->keyBinds.restart = atoi(value);
    }
    else if (MATCH("Keybinds", "screenshot")) {
        pconfig->keyBinds.screenshot = atoi(value);
    }
    else {
        return 0;
    }

    return 1;
}
