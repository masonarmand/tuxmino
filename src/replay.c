#include "tuxmino.h"
#include <string.h>

Timer inputTimer;
double frameTime;
FILE *fp;

void initRecording(void)
{
    fp = fopen( "file.txt" , "w" );
}

void recordInputs(void)
{
    if (inputTimer.startTime == 0)
        startTimer(&inputTimer, framesToMilliseconds(1));

    if (TimerDone(inputTimer)) {
        resetTimer(&inputTimer);
        startTimer(&inputTimer, framesToMilliseconds(1));
        int pressed = GetKeyPressed();
        printf("%i\n", pressed);
        char str[4];
        sprintf(str, "%i", pressed);
        fwrite(str, 1 , strlen(str) , fp );
    }
}

void exitRecording(void)
{
    fclose(fp);
}
