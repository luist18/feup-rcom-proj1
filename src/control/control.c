#include "control.h"

#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include "../packet/packet.h"
#include "../util/flags.h"

int retries = 0;
int flag = 0;

void handle_ua_state(enum STATE_UA *current_state, char *byte, control_packet *packet) {
    switch (*current_state) {
        case START:
            *current_state = *byte == DELIMITER_FLAG ? FLAG_RCV : START;
            break;

        case FLAG_RCV:
            *current_state = *byte == RECEPTOR_ADDRESS ? A_RCV : START;
            break;

        case A_RCV:
            if (*byte == DELIMITER_FLAG)
                *current_state = FLAG_RCV;
            else if (*byte == CONTROL_UA)
                *current_state = C_RCV;
            else
                *current_state = START;

            break;

        case C_RCV:
            if (*byte == (CONTROL_UA ^ RECEPTOR_ADDRESS))
                *current_state = BCC_OK;
            else if (*byte == DELIMITER_FLAG)
                *current_state = FLAG_RCV;
            else
                *current_state = START;

            break;

        case BCC_OK:
            *current_state = *byte == DELIMITER_FLAG ? STOP : START;
            break;

        default:
            break;
    }
}

void alarm_handler(int sig) {
    flag = 1;
    retries++;

    // This prevents from printing the last one
    if (retries <= MAX_TIMEOUT_RETRIES)
        printf("Retrying to send packet... (%d/%d)\n", retries, MAX_TIMEOUT_RETRIES);
}

void setup_handler() {
    (void)signal(SIGALRM, alarm_handler);
}

int start_emitter(int filedes) {
    printf("Establishing connection with receptor...\n");

    enum STATE_UA state = START;

    control_packet packet = build_control_packet(EMITTER_ADDRESS, CONTROL_SET);

    do {
        send_control_packet(filedes, packet);

        alarm(3);

        char byte;

        flag = 0;

        while (!flag && state != STOP) {
            read(filedes, &byte, sizeof(byte));

            handle_ua_state(&state, &byte, &packet);
        }
    } while (flag && retries <= MAX_TIMEOUT_RETRIES);

    if (state != STOP) {
        printf("Connection failed: timed out!\n");
        return -1;
    }

    return 0;
}