#include "packet_al.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../util/flags.h"

char* build_app_control_packet(char control_byte, unsigned char type, unsigned char size,
                               char* data, int* packet_length) {
    unsigned int length = 3 + (unsigned int)size;

    char* buffer = malloc((int)length * sizeof(char));

    buffer[0] = control_byte;
    buffer[1] = type;
    buffer[2] = size;

    for (int i = 0; i < size; ++i)
        buffer[3 + i] = data[i];

    *packet_length = length;
    return buffer;
}

char* append_app_control_packet(char* buffer, unsigned char type, unsigned char size,
                                char* data, int old_length, int* packet_length) {
    unsigned int length = old_length + 2 + (unsigned int)size;

    buffer = realloc(buffer, (int)length * sizeof(char));

    buffer[old_length] = type;
    buffer[old_length + 1] = size;

    for (int i = 0; i < size; ++i)
        buffer[old_length + 2 + i] = data[i];

    *packet_length = length;
    return buffer;
}

char* build_app_data_packet(unsigned char sequence_number, unsigned int size, char* data, unsigned int* packet_length) {
    int length = 4 + size;

    char* packet = malloc(length);

    packet[0] = AL_CONTROL_DATA;
    packet[1] = sequence_number % 256;
    packet[2] = size / 256;
    packet[3] = size % 256;

    memcpy(&packet[4], data, size);

    *packet_length = length;

    return packet;
}