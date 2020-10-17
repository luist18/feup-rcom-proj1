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
enum STATE_UA {
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK,
    STOP
};

/**
 * @brief Handles every transition in the state machine.
 * 
 * @param current_state the current state
 * @param byte          the byte received
 * @param packet        the packet sent
 */
void handle_ua_state(enum STATE_UA *current_state, char *byte, control_packet *packet);

/**
 * @brief Sets up the alarm handler. 
 */
void setup_handler();

#endif
