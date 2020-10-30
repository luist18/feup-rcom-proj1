#include "packet_dl.h"

#include <string.h>
#include <unistd.h>

#include "../../util/flags.h"
#include "../../util/util.h"

control_packet build_control_packet(char address, char control) {
    control_packet packet = {.I_FLAG = DELIMITER_FLAG,
                             .ADDRESS = address,
                             .CONTROL = control,
                             .PROTECTION_FIELD = address ^ control,
                             .F_FLAG = DELIMITER_FLAG};

    return packet;
}

unsigned char *build_information_packet(char *data, unsigned int length, unsigned int sequence_number, unsigned int *packet_length) {
    unsigned char *packet = malloc(INFORMATION_PACKET_BASE_SIZE + length);
    unsigned char *stuffed_packet = malloc(INFORMATION_PACKET_BASE_SIZE + length);

    packet[0] = DELIMITER_FLAG;
    packet[1] = EMITTER_ADDRESS;
    packet[2] = sequence_number << 6;
    packet[3] = packet[2] ^ EMITTER_ADDRESS;
    memcpy(&packet[4], data, length);
    packet[length + 4] = get_data_bcc(data, length);
    packet[length + 5] = DELIMITER_FLAG;

    unsigned int new_length = stuff(packet, stuffed_packet, INFORMATION_PACKET_BASE_SIZE + length);

    free(packet);

    *packet_length = new_length;

    return stuffed_packet;
}

unsigned int stuff(unsigned char *data, unsigned char *packet, unsigned int length) {
    unsigned int new_length = (unsigned int)length;
    int current_index = 1;

    packet[0] = data[0];

    for (int i = 1; i < length - 1; ++i) {
        char byte = data[i];

        if (byte == DELIMITER_FLAG) {
            packet = realloc(packet, ++new_length);

            packet[current_index++] = ESCAPE;
            packet[current_index] = DELIMITER_FLAG ^ 0x20;
        } else if (byte == ESCAPE) {
            packet = realloc(packet, ++new_length);

            packet[current_index++] = ESCAPE;
            packet[current_index] = ESCAPE ^ 0x20;
        } else {
            packet[current_index] = byte;
        }

        current_index++;
    }

    packet[current_index] = data[length - 1];

    return new_length;
}

unsigned char get_data_bcc(char *data, unsigned int length) {
    unsigned char bcc = data[0];

    for (int i = 1; i < length; ++i)
        bcc ^= data[i];

    return bcc;
}
