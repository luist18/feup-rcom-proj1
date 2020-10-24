#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#include "../app_layer/app_layer.h"
#include "../app_layer/packet/packet_al.h"
#include "../data_link/data_link.h"
#include "../util/file.h"
#include "../util/flags.h"
#include "../util/util.h"

#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */

int send_file(int fd, char *filename, long file_length);

int main(int argc, char **argv) {
    // TODO: remove serial port ttyS1[0, 1]. Only for virtual port test purposes.
    if ((argc < 3) ||
        ((strcmp("/dev/ttyS0", argv[1]) != 0) &&
         (strcmp("/dev/ttyS1", argv[1]) != 0) &&
         (strcmp("/dev/ttyS10", argv[1]) != 0) &&
         (strcmp("/dev/ttyS11", argv[1]) != 0))) {
        printf("Usage:\temitter SerialPort Filename\n\tex: emitter /dev/ttyS0 filename.jpg\n");
        exit(1);
    }

    long file_length = get_file_length(argv[2]);

    if (file_length < 0) {
        printf("The file %s does not exist or something unexpected occurred!\n", argv[2]);
        exit(2);
    }

    int fd;
    if (fd = llopen(argv[1], EMITTER), fd < 0)
        exit(3);

    if (send_file(fd, argv[2], file_length) < 0) {
        printf("Something unexpected occurred while sending the file!\n");

        if (llclose(fd, EMITTER) < 0)
            exit(5);

        exit(4);
    }

    if (llclose(fd, EMITTER) < 0)
        exit(5);

    return 0;
}

int send_file(int fd, char *filename, long file_length) {
    /* 1. Sending start packet */
    unsigned char length_buffer_size;
    char *length_buffer = length_to_array(file_length, &length_buffer_size);

    int control_packet_length;
    char *control_packet = build_app_control_packet(AL_CONTROL_START, AL_TYPE_LENGTH, length_buffer_size, length_buffer, &control_packet_length);
    control_packet = append_app_control_packet(control_packet, AL_TYPE_NAME, strlen(filename), filename, control_packet_length, &control_packet_length);

    if (llwrite(fd, control_packet, control_packet_length) < 0) {
        printf("Could not send start app packet!\n");
        return -1;
    }

    /* 2. Sending data packets */

    /* 3. Sending end packet */
    control_packet = build_app_control_packet(AL_CONTROL_END, AL_TYPE_LENGTH, length_buffer_size, length_buffer, &control_packet_length);
    control_packet = append_app_control_packet(control_packet, AL_TYPE_NAME, strlen(filename), filename, control_packet_length, &control_packet_length);

    if (llwrite(fd, control_packet, control_packet_length) < 0) {
        printf("Could not send end app packet!\n");
        return -1;
    }

    return 0;
}