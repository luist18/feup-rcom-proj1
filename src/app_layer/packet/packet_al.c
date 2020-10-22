#include "packet_al.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../util/flags.h"

char* build_app_control_packet(char control_byte, char type[], unsigned char sizes[],
                               char** data, unsigned int size, unsigned int* packet_length) {
    int current_length = 2 * sizeof(char), index = 0;

    char* packet = malloc(current_length);

    packet[index++] = control_byte;

    for (int i = 0; i < size; ++i) {
        char data_type = type[i];
        char data_size = sizes[i];

        current_length += (data_size + 2);

        packet = realloc(packet, current_length);

        packet[index++] = data_type;
        packet[index++] = data_size;

        memcpy(&packet[index], &(*data[i]), data_size);

        index += data_size;
    }

    *packet_length = current_length - 1;

    return packet;
}

char* build_app_data_packet(char sequence_number, unsigned int size, char* data, unsigned int* packet_length) {
    int length = 4 + size;

    char* packet = malloc(length);

    packet[0] = AL_CONTROL_DATA;
    packet[1] = sequence_number % 255;
    packet[2] = size / 256;
    packet[3] = size % 256;

    memcpy(&packet[4], data, size);

    *packet_length = length;

    return packet;
}