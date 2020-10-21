#ifndef DATA_LINK_PACKET_H
#define DATA_LINK_PACKET_H

#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Represents a control packet.
 */
typedef struct control_packet {
    unsigned char I_FLAG : 8;
    unsigned char ADDRESS : 8;
    unsigned char CONTROL : 8;
    unsigned char PROTECTION_FIELD : 8;
    unsigned char F_FLAG : 8;
} control_packet;

/**
 * @brief Builds a control packet from an address and control.
 * 
 * @param address           the address
 * @param control           the control type
 * @return control_packet   the control packet
 */
control_packet build_control_packet(char address, char control);

unsigned char *build_information_packet(char *data, unsigned int length, unsigned int sequence_number, unsigned int *packet_length);

/**
 * @brief Stuffs data.
 * 
 * @param data              the data to stuff
 * @param length            the length of the data
 * @param new_length        the length of the stuffed data
 * @return unsigned char*   the stuffed data
 */
unsigned char *stuff(char *data, unsigned int length, unsigned int *new_length);

char *destuff(char *data, unsigned int length, unsigned int *new_length);

unsigned char get_data_bcc(char *data, unsigned int length);

#endif