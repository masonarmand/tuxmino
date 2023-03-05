/*
 * File: config.c
 * --------------
 *
 * Original Author: Mason Armand
 * Last Modified: Feb 14, 2023
 * Last Modified: Feb 14, 2023
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
        pconfig->KeyBinds.ccw = atoi(value);
    }
    else if (MATCH("Keybinds", "counter-clockwise-alt")) {
        pconfig->KeyBinds.ccwAlt = atoi(value);
    }
    else if (MATCH("Keybinds", "clockwise")) {
        pconfig->KeyBinds.cw = atoi(value);
    }
    else if (MATCH("Keybinds", "swap-hold")) {
        pconfig->KeyBinds.swapHold = atoi(value);
    }
    else if (MATCH("Keybinds", "sonic-drop")) {
        pconfig->KeyBinds.sonicDrop = atoi(value);
    }
    else if (MATCH("Keybinds", "move-left")) {
        pconfig->KeyBinds.left = atoi(value);
    }
    else if (MATCH("Keybinds", "move-right")) {
        pconfig->KeyBinds.right = atoi(value);
    }
    else if (MATCH("Keybinds", "move-down")) {
        pconfig->KeyBinds.down = atoi(value);
    }
    else if (MATCH("Keybinds", "move-up")) {
        pconfig->KeyBinds.up = atoi(value);
    }
    else if (MATCH("Keybinds", "ui-select")) {
        pconfig->KeyBinds.uiSelect = atoi(value);
    }
    else {
        return 0;
    }

    return 1;
}
