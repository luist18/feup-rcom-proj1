#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>

#include "../app_layer/app_layer.h"
#include "../data_link/data_link.h"

#define _POSIX_SOURCE 1 /* POSIX compliant source */

int main(int argc, char** argv) {
    if ((argc < 2) ||
        ((strcmp("/dev/ttyS0", argv[1]) != 0) &&
         (strcmp("/dev/ttyS1", argv[1]) != 0) &&
         (strcmp("/dev/ttyS10", argv[1]) != 0) &&
         (strcmp("/dev/ttyS11", argv[1]) != 0))) {
        printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
        exit(1);
    }

    int fd;
    if (fd = llopen(argv[1], RECEPTOR), fd < 0)
        exit(1);

    if (receive_file(fd) < 0) {
        printf("Something went wrong while receiving the file!\n");
        exit(3);
    }

    if (llclose(fd, RECEPTOR) < 0)
        exit(2);

    return 0;
}
