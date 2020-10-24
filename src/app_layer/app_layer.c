#include "app_layer.h"

#include <string.h>

#include "../app_layer/packet/packet_al.h"
#include "../data_link/data_link.h"
#include "../util/flags.h"
#include "../util/util.h"

int al_send_packet(int fd, char *packet, unsigned int size) {
    for (int i = 0; i < size; ++i) {
        printf("%02x", packet[i]);
    }

    printf("\n");

    return llwrite(fd, packet, size);
}

int send_control_packet(int fd, char *filename, long file_length, char control_byte) {
    unsigned char length_buffer_size;
    char *length_buffer = length_to_array(file_length, &length_buffer_size);

    int control_packet_length;
    char *control_packet = build_app_control_packet(control_byte, AL_TYPE_LENGTH, length_buffer_size, length_buffer, &control_packet_length);
    control_packet = append_app_control_packet(control_packet, AL_TYPE_NAME, strlen(filename) + 1, filename, control_packet_length, &control_packet_length);

    if (llwrite(fd, control_packet, control_packet_length) < 0) {
        printf("Could not send %s app packet!\n", control_byte == AL_CONTROL_START ? "start" : "start");
        return -1;
    }

    return 0;
}

int send_data_packets(int fd, char *filename, long file_length) {
    FILE *file;

    file = fopen(filename, "rb");
    if (file == NULL) {
        printf("Error while opening file!\n");
        exit(6);
    }

    int bytes_sent = 0, bytes_read = 0, sequence_number = 0;

    char buffer[AL_PACKET_SIZE];

    while (bytes_sent != file_length) {
        int content_size = file_length - bytes_sent >= 0 ? AL_PACKET_SIZE : file_length - bytes_sent;

        bytes_read = fread(buffer, 1, content_size, file);

        unsigned int packet_length;
        char *packet = build_app_data_packet(sequence_number++, bytes_read, buffer, &packet_length);

        if (al_send_packet(fd, packet, packet_length) < 0) return -1;

        bytes_sent += content_size;
    }

    return 0;
}

int send_file(int fd, char *filename, long file_length) {
    if (send_control_packet(fd, filename, file_length, AL_CONTROL_START) < 0)
        return -1;

    if (send_data_packets(fd, filename, file_length) < 0)
        return -1;

    if (send_control_packet(fd, filename, file_length, AL_CONTROL_END) < 0)
        return -1;

    return 0;
}