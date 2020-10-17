#include "packet.h"

#include <string.h>
#include <unistd.h>

#include "../../util/flags.h"

control_packet build_control_packet(char address, char control) {
    control_packet packet = {.I_FLAG = DELIMITER_FLAG,
                             .ADDRESS = address,
                             .CONTROL = control,
                             .PROTECTION_FIELD = address ^ control,
                             .F_FLAG = DELIMITER_FLAG};

    return packet;
}

unsigned char *build_information_packet(char *data, unsigned int length, unsigned int sequence_number) {
    unsigned int new_length;

    unsigned char *data_stuffed = stuff(data, length, &new_length);

    unsigned char *packet = malloc(INFORMATION_PACKET_BASE_SIZE + new_length);

    packet[0] = DELIMITER_FLAG;
    packet[1] = EMITTER_ADDRESS;
    packet[2] = sequence_number << 6;
    packet[3] = DELIMITER_FLAG ^ EMITTER_ADDRESS;
    memcpy(&packet[4], data_stuffed, new_length);
    packet[new_length + 4] = get_data_bcc(data, length);
    packet[new_length + 5] = DELIMITER_FLAG;

    return packet;
}

unsigned char *stuff(char *data, unsigned int length, unsigned int *new_length) {
    unsigned char *stuffed_data = malloc(length * sizeof(char));

    *new_length = 0;
    unsigned int offset = 0;

    for (int i = 0; i < length; ++i) {
        char byte = data[i];

        stuffed_data[*new_length] = data[i];

        if (byte == DELIMITER_FLAG || byte == ESCAPE) {
            stuffed_data[(*new_length)++] = ESCAPE;

            offset++;

            stuffed_data = realloc(stuffed_data, length + offset);
        }

        (*new_length)++;
    }

    return stuffed_data;
}

unsigned char get_data_bcc(char *data, unsigned int length) {
    unsigned char bcc = data[0];

    for (int i = 1; i < length; ++i)
        bcc ^= data[i];

    return bcc;
}
