/*
 * File: timings.c
 * ---------------
 * Functions for calculating various timings such
 * as gravity and frame time.
 *
 * Original Author: Mason Armand
 * Contributors:
 * Date Created: Oct 18, 2022
 * Last Modified: Feb 6, 2023
 */

#include "tuxmino.h"

double framesToMilliseconds(int frames) {
    return frames * 0.016f;//0.0166670f;
}

double gravityLevelToMilliseconds(int gravity) {
    double frameTime = 0.0166670f;
    return (frameTime * 256) / gravity;
}
