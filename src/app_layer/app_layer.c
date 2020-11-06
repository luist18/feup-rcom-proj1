#include "app_layer.h"

#include <string.h>

#include "../app_layer/packet/packet_al.h"
#include "../data_link/data_link.h"
#include "../util/flags.h"
#include "../util/util.h"

int al_send_packet(int fd, char *packet, unsigned int size) {
    return llwrite(fd, packet, size);
}

int send_control_packet(int fd, char *filename, long file_length, char control_byte) {
    unsigned char length_buffer_size;
    char *length_buffer = length_to_array(file_length, &length_buffer_size);

    int control_packet_length;
    char *control_packet = build_app_control_packet(control_byte, AL_TYPE_LENGTH, length_buffer_size, length_buffer, &control_packet_length);
    control_packet = append_app_control_packet(control_packet, AL_TYPE_NAME, strlen(filename) + 1, filename, control_packet_length, &control_packet_length);

    if (llwrite(fd, control_packet, control_packet_length) < 0) {
        printf("Could not send %s app packet!\n", control_byte == AL_CONTROL_START ? "start" : "end");
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

    int bytes_sent = 0, bytes_read = 0;
    unsigned char sequence_number = 0;

    char buffer[AL_PACKET_SIZE];

    double start_time = get_current_time();

    int content_size = file_length < AL_PACKET_SIZE ? file_length : AL_PACKET_SIZE;
    while (bytes_sent != file_length) {
        if (bytes_sent + content_size > file_length)
            content_size = file_length - bytes_sent;

        bytes_read = fread(buffer, 1, content_size, file);

        unsigned int packet_length;
        char *packet = build_app_data_packet(sequence_number++, bytes_read, buffer, &packet_length);

        if (al_send_packet(fd, packet, packet_length) < 0)
            return -1;

        bytes_sent += content_size;

        print_progress_bar(bytes_sent, file_length, start_time);
    }

    return 0;
}

int send_file(int fd, char *filename, long file_length) {
    if (send_control_packet(fd, filename, file_length, AL_CONTROL_START) < 0)
        return -1;

    printf("Application start packet sent.\n");

    if (send_data_packets(fd, filename, file_length) < 0)
        return -1;

    printf("Application data packets sent.\n");

    if (send_control_packet(fd, filename, file_length, AL_CONTROL_END) < 0)
        return -1;

    printf("Application end packet sent.\n");
    return 0;
}

int read_control_packet(int fd, char *filename, long *file_length) {
    char *buffer = malloc(sizeof(char));
    int bytes_read = llread(fd, buffer);

    int index = 1;

    while (index != bytes_read) {
        char type_byte = buffer[index++];
        char length = buffer[index++];

        if (type_byte == AL_TYPE_LENGTH) {
            char length_buffer[length];
            memcpy(length_buffer, &buffer[index], length);

            *file_length = array_to_length(length_buffer, length);
        } else if (type_byte == AL_TYPE_NAME) {
            filename = realloc(filename, length);

            strncpy(filename, &buffer[index], length);
        }

        index += length;
    }

    free(buffer);

    return 0;
}

int read_data_packet(int fd, char *buffer, unsigned int *data_length, unsigned char *sequence_number) {
    int bytes_read = llread(fd, buffer);

    unsigned char tmp_sequence_number = (unsigned char)buffer[1];
    unsigned char l2 = (unsigned char)buffer[2];
    unsigned char l1 = (unsigned char)buffer[3];

    *data_length = 256 * l2 + l1;
    *sequence_number = tmp_sequence_number;

    memcpy(buffer, &(buffer[4]), *data_length);

    return bytes_read;
}

int read_data_packets(int fd, char *filename, long file_length) {
    FILE *file;
    file = fopen(filename, "wb");

    unsigned char sequence_number = 0;
    long total_bytes_read = 0;
    unsigned int data_bytes_read;

    int i = 0;

    double start_time = get_current_time();

    while (total_bytes_read != file_length) {
        unsigned char tmp_sequence_number;

        char data[PACKET_SIZE];

        read_data_packet(fd, data, &data_bytes_read, &tmp_sequence_number);

        total_bytes_read += data_bytes_read;

        if (sequence_number != tmp_sequence_number) {
            fclose(file);
            return -1;
        }

        fwrite(data, data_bytes_read, 1, file);

        sequence_number++;
        sequence_number %= 256;

        print_progress_bar(total_bytes_read, file_length, start_time);

        ++i;
    }

    if (total_bytes_read != file_length)
        return -1;

    fclose(file);

    return 0;
}

int receive_file(int fd) {
    char *filename_start = malloc(sizeof(char));
    long file_length;

    if (read_control_packet(fd, filename_start, &file_length) < 0) {
        free(filename_start);
        return -1;
    }

    printf("Application start packet received.\n");
    printf("\tFile size: %ld bytes\n\tFile name: %s\n", file_length, filename_start);

    if (read_data_packets(fd, filename_start, file_length) < 0) {
        free(filename_start);
        return -1;
    }

    free(filename_start);

    printf("Application data packets received.\n");

    char *filename_end = malloc(sizeof(char));

    if (read_control_packet(fd, filename_end, &file_length) < 0) {
        free(filename_end);
        return -1;
    }

    free(filename_end);

    printf("Application end packet received.\n");

    return 0;
}