/**
 * @file control.h
 * @brief Functions related to state and timeout handling.
 */
#ifndef CONTROLL_H
#define CONTROLL_H

#include <stdio.h>
#include <stdlib.h>

#include "../packet/packet_dl.h"

/**
 * @brief Enumerate for the UA state machine.
 */
enum STATE {
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK,
    STOP,
    RR_RCV,
    REJ_RCV
};

/**
 * @Brief Enumerate for the reading Information state machine
 */
enum INFO_STATE {
    INFO_START,
    INFO_FLAG_RCV,
    INFO_A_RCV,
    INFO_C_RCV,
    INFO_BCC1_OK,
    INFO_DATA_START,
    INFO_DATA_CONTINUE,
    INFO_STOP
};

/**
 * @brief Handles every transition in the state machine.
 * 
 * @param current_state the current state
 * @param byte          the byte received
 * @param address       the address
 * @param control_flag  the control flag
 */
void handle_state(enum STATE *current_state, char *byte, char address, char control_flag);

void handle_state_emitter(enum STATE *current_state, char *byte, char control_rr, char control_rej);

void handle_state_receptor_information(enum INFO_STATE *current_state, char *byte, char address, char control_flag);

/**
 * @brief Sets up the alarm handler. 
 */
void setup_handler();

#endif
