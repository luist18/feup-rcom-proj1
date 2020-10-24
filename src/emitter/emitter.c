#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#include "../app_layer/app_layer.h"
#include "../data_link/data_link.h"
#include "../util/file.h"
#include "../util/flags.h"

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
