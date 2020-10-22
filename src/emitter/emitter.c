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

#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */

int send_file(int fd, char *data, char *filename, int file_length);

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

    char *file_data = "";
    long file_length = read_file(argv[2], file_data);

    if (file_length < 0) {
        printf("The file %s does not exist or something unexpected occurred!\n", argv[2]);
        exit(2);
    }

    int fd;
    if (fd = llopen(argv[1], EMITTER), fd < 0)
        exit(3);

    if (send_file(fd, file_data, argv[2], file_length) < 0) {
        printf("Something unexpected while sending the file!\n");
        exit(4);
    }

    if (llclose(fd, EMITTER) < 0)
        exit(5);

    return 0;
}

int send_file(int fd, char *data, char *filename, int file_length) {
    int bytes_sent = 0;
    int sequence_number = 0;

    while (bytes_sent != file_length) {
        int content_size = file_length - bytes_sent >= 0 ? AL_PACKET_SIZE : file_length - bytes_sent;

        char content[content_size];

        memcpy(content, &data[bytes_sent], content_size);

        unsigned int packet_length;
        char *packet = build_app_data_packet(sequence_number, content_size, content, &packet_length);

        if (al_send_packet(fd, packet, packet_length) < 0) return -1;

        bytes_sent += content_size;
    }

    return 0;
}