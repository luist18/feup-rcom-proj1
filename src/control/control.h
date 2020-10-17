#include <stdio.h>
#include <stdlib.h>

void setup_handler();

int start_emitter(int filedes);

enum STATE_UA {
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK,
    STOP
};
