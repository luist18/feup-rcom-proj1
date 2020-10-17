#ifndef DATA_LINK_PACKET_H
#define DATA_LINK_PACKET_H

#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Represents a control packet.
 */
typedef struct control_packet {
    char I_FLAG : 8;
    char ADDRESS : 8;
    char CONTROL : 8;
    char PROTECTION_FIELD : 8;
    char F_FLAG : 8;
} control_packet;

/**
 * @brief Builds a control packet from an address and control.
 * 
 * @param address           the address
 * @param control           the control type
 * @return control_packet   the control packet
 */
control_packet build_control_packet(char address, char control);

#endif