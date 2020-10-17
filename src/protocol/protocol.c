#include "protocol.h"

#include <fcntl.h>
#include <strings.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>

#include "../control/control.h"

struct termios oldtio, newtio;

int llopen(char *port, enum open_type open_type) {
    int return_code, fd;

    switch (open_type) {
        case EMITTER:
            /*
             * Open serial port device for reading and writing and not as controlling tty
             * because we don't want to get killed if linenoise sends CTRL-C.
             */
            fd = open(port, O_RDWR | O_NOCTTY);
            if (fd < 0) {
                perror(port);
                return -1;
            }

            if (tcgetattr(fd, &oldtio) == -1) {
                perror("tcgetattr");
                return -1;
            }

            bzero(&newtio, sizeof(newtio));
            newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
            newtio.c_iflag = IGNPAR;
            newtio.c_oflag = 0;

            newtio.c_lflag = 0;

            newtio.c_cc[VTIME] = 0;
            newtio.c_cc[VMIN] = 0;

            tcflush(fd, TCIOFLUSH);

            if (tcsetattr(fd, TCSANOW, &newtio) == -1) {
                perror("tcsetattr");
                return -1;
            }

            setup_handler();

            if (start_emitter(fd) < 0)
                return -1;

            printf("Connection established.\n");
            return_code = fd;
            break;

        default:
            return_code = -1;
            break;
    }

    return return_code;
}

int llclose(int fd) {
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1) {
        perror("tcsetattr");
        return -1;
    }

    close(fd);

    return 0;
}