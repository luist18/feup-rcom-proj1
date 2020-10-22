#include "control.h"

#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include "../../util/flags.h"
#include "../packet/packet_dl.h"

extern int retries;
extern int flag;

void handle_state_receptor_information(enum INFO_STATE *current_state, char *byte, char address, char sequence_byte) {
    switch (*current_state) {
        case INFO_START:
            *current_state = *byte == DELIMITER_FLAG ? INFO_FLAG_RCV : INFO_START;
            break;

        case INFO_FLAG_RCV:
            if (*byte == DELIMITER_FLAG)
                *current_state = INFO_FLAG_RCV;
            else
                *current_state = *byte == address ? INFO_A_RCV : INFO_START;
            break;

        case INFO_A_RCV:
            if (*byte == DELIMITER_FLAG)
                *current_state = INFO_FLAG_RCV;
            else if (*byte == sequence_byte)
                *current_state = INFO_C_RCV;
            else
                *current_state = INFO_START;

            break;

        case INFO_C_RCV:
            if (*byte == (address ^ sequence_byte))
                *current_state = INFO_BCC1_OK;
            else if (*byte == DELIMITER_FLAG)
                *current_state = INFO_FLAG_RCV;
            else
                *current_state = INFO_START;

            break;

        case INFO_BCC1_OK:
            *current_state = *byte == DELIMITER_FLAG ? INFO_FLAG_RCV : INFO_DATA_START;
            break;

        case INFO_DATA_START:
            *current_state = *byte == DELIMITER_FLAG ? INFO_FLAG_RCV : INFO_DATA_CONTINUE;
            break;

        case INFO_DATA_CONTINUE:
            *current_state = *byte == DELIMITER_FLAG ? INFO_STOP : INFO_DATA_CONTINUE;
            break;

        default:
            break;
    }
}

void handle_state(enum STATE *current_state, char *byte, char address, char control_flag) {
    switch (*current_state) {
        case START:
            *current_state = *byte == DELIMITER_FLAG ? FLAG_RCV : START;
            break;

        case FLAG_RCV:
            if (*byte == DELIMITER_FLAG)
                *current_state = FLAG_RCV;
            else
                *current_state = *byte == address ? A_RCV : START;
            break;

        case A_RCV:
            if (*byte == DELIMITER_FLAG)
                *current_state = FLAG_RCV;
            else if (*byte == control_flag)
                *current_state = C_RCV;
            else
                *current_state = START;

            break;

        case C_RCV:
            if (*byte == (address ^ control_flag))
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

void handle_state_emitter(enum STATE *current_state, char *byte, char control_rr, char control_rej) {
    switch (*current_state) {
        case START:
            *current_state = *byte == DELIMITER_FLAG ? FLAG_RCV : START;
            break;

        case FLAG_RCV:
            if (*byte == DELIMITER_FLAG)
                *current_state = FLAG_RCV;
            else
                *current_state = *byte == EMITTER_ADDRESS ? A_RCV : START;
            break;

        case A_RCV:
            if (*byte == DELIMITER_FLAG)
                *current_state = FLAG_RCV;
            else if (*byte == control_rr)
                *current_state = RR_RCV;
            else if (*byte == control_rej)
                *current_state = REJ_RCV;
            else
                *current_state = START;

            break;

        case RR_RCV:
            if (*byte == (EMITTER_ADDRESS ^ control_rr))
                *current_state = BCC_OK;
            else if (*byte == DELIMITER_FLAG)
                *current_state = FLAG_RCV;
            else
                *current_state = START;

            break;

        case REJ_RCV:
            if (*byte == (EMITTER_ADDRESS ^ control_rej))
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
