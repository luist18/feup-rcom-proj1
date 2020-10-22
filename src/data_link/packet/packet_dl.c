#include "packet_dl.h"

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

unsigned char *build_information_packet(char *data, unsigned int length, unsigned int sequence_number, unsigned int *packet_length) {
    unsigned int new_length;

    unsigned char *data_stuffed = stuff(data, length, &new_length);

    unsigned char *packet = malloc(INFORMATION_PACKET_BASE_SIZE + new_length);

    packet[0] = DELIMITER_FLAG;
    packet[1] = EMITTER_ADDRESS;
    packet[2] = sequence_number << 6;
    packet[3] = packet[2] ^ EMITTER_ADDRESS;
    memcpy(&packet[4], data_stuffed, new_length);
    packet[new_length + 4] = get_data_bcc(data, length);
    packet[new_length + 5] = DELIMITER_FLAG;

    *packet_length = new_length + 6;

    return packet;
}

unsigned char *stuff(char *data, unsigned int length, unsigned int *new_length) {
    unsigned char *stuffed_data = malloc(length * sizeof(char));

    *new_length = 0;
    unsigned int offset = 0;

    for (int i = 0; i < length; ++i) {
        char byte = data[i];

        if (byte == DELIMITER_FLAG) {
            stuffed_data[*new_length] = ESCAPE;
            stuffed_data[(*new_length)++] = DELIMITER_FLAG ^ 0x20;

            offset++;

            stuffed_data = realloc(stuffed_data, length + offset);
        } else if (byte == ESCAPE) {
            stuffed_data[*new_length] = ESCAPE;
            stuffed_data[(*new_length)++] = ESCAPE ^ 0x20;

            offset++;

            stuffed_data = realloc(stuffed_data, length + offset);
        } else {
            stuffed_data[*new_length] = byte;
        }

        (*new_length)++;
    }

    return stuffed_data;
}

char *destuff(char *data, unsigned int length, unsigned int *new_length) {
    char *destuffed_data = malloc(length * sizeof(char));

    for (int i = 0; i < 4; i++)
        destuffed_data[i] = data[i];

    char byte;

    int current_index = 4;

    for (int i = 4; i < length - 2; ++i, ++current_index) {
        byte = data[i];

        if (byte == ESCAPE) {
            if (data[i + 1] == 0x5e) {
                destuffed_data[current_index] = 0x7e;
                i++;
            } else if (data[i + 1] == 0x5d) {
                destuffed_data[current_index] = ESCAPE;
                i++;
            }
        } else {
            destuffed_data[current_index] = byte;
        }
    }

    destuffed_data[current_index++] = data[length - 2];
    destuffed_data[current_index++] = data[length - 1];

    destuffed_data = realloc(destuffed_data, current_index * sizeof(char));

    *new_length = current_index;

    return destuffed_data;
}

unsigned char get_data_bcc(char *data, unsigned int length) {
    unsigned char bcc = data[0];

    for (int i = 1; i < length; ++i)
        bcc ^= data[i];

    return bcc;
}
