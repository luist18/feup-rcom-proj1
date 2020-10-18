/**
 * @file control.h
 * @brief Functions related to state and timeout handling.
 */
#ifndef CONTROLL_H
#define CONTROLL_H

#include <stdio.h>
#include <stdlib.h>

#include "../packet/packet.h"

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
 * @brief Handles every transition in the state machine.
 * 
 * @param current_state the current state
 * @param byte          the byte received
 * @param address       the address
 * @param control_flag  the control flag
 */
void handle_state(enum STATE *current_state, char *byte, char address, char control_flag);

void handle_state_emitter(enum STATE *current_state, char *byte, char control_rr, char control_rej);

/**
 * @brief Sets up the alarm handler. 
 */
void setup_handler();

#endif
