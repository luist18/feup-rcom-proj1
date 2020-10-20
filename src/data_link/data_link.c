#include "data_link.h"

#include <fcntl.h>
#include <strings.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>

#include "../util/flags.h"
#include "control/control.h"
#include "packet/packet.h"

struct termios oldtio, newtio;

int retries = 0;
int flag = 0;
int sequence_number = 0;

int llopen(char *port, enum open_type open_type) {
    int return_code, fd;

    switch (open_type) {
        case EMITTER:
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

            if (llopen_emitter(fd) < 0)
                return -1;

            printf("Connection established.\n");

            return_code = fd;

            break;

        case RECEPTOR:
            fd = open(port, O_RDWR | O_NOCTTY);
            if (fd < 0) {
                perror(port);
                exit(-1);
            }

            if (tcgetattr(fd, &oldtio) == -1) {
                perror("tcgetattr");
                exit(-1);
            }

            bzero(&newtio, sizeof(newtio));
            newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
            newtio.c_iflag = IGNPAR;
            newtio.c_oflag = 0;

            /* set input mode (non-canonical, no echo,...) */
            newtio.c_lflag = 0;

            newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
            newtio.c_cc[VMIN] = 5;  /* blocking read until 5 chars received */

            tcflush(fd, TCIOFLUSH);

            if (tcsetattr(fd, TCSANOW, &newtio) == -1) {
                perror("tcsetattr");
                exit(-1);
            }

            setup_handler();

            if (llopen_receptor(fd) < 0)
                return -1;

            return_code = fd;

            break;

        default:
            return_code = -1;
            break;
    }

    return return_code;
}

int llread(int fd, char *buffer) {
    enum INFO_STATE state = INFO_START;

    char *data = malloc(sizeof(char));
    unsigned int byte_count = 0;

    char byte;

    while (state != INFO_STOP) {
        read(fd, &byte, sizeof(byte));
        data[byte_count++] = byte;
        data = realloc(data, byte_count + 1);

        handle_state_receptor_information(&information_state, &byte, EMITTER_ADDRESS, INFO_SEQUENCE(sequence_number));
    }

    char data_bcc2 = data[byte_count - 2];

    unsigned int new_length;
    data = destuff(data, byte_count, &new_length);

    char bcc2 = data[4];
    for (int i = 5; i < new_length - 2; i++) {
        bcc2 ^= data[i];
    }

    int data_sequence_number = data[2] >> 6;

    if (data_sequence_number == sequence_number && data_bcc2 == bcc2) {
        control_packet rr_packet = build_control_packet(EMITTER_ADDRESS, CONTROL_RR(sequence_number));

        write(fd, &rr_packet, sizeof(rr_packet));

        sequence_number = !sequence_number;
    } else {
        control_packet rej_packet = build_control_packet(EMITTER_ADDRESS, CONTROL_REJ(sequence_number));

        write(fd, &rej_packet, sizeof(rej_packet));
    }

    return byte_count;
}

int llwrite(int filedes, char *data, int length) {
    retries = 0;

    enum STATE state = START;
    enum STATE control;

    unsigned char *packet = build_information_packet(data, length, sequence_number);

    do {
        write(filedes, &packet, sizeof(packet));

        alarm(3);

        char byte;

        flag = 0;

        while (!flag && state != STOP) {
            read(filedes, &byte, sizeof(byte));

            handle_state_emitter(&state, &byte, CONTROL_RR(sequence_number), CONTROL_REJ(sequence_number));

            if (state == RR_RCV || state == REJ_RCV)
                control = state;
        }
    } while (flag && retries <= MAX_TIMEOUT_RETRIES);

    if (state != STOP) {
        printf("Connection failed: timed out!\n");
        return -1;
    }

    if (control == RR_RCV)
        sequence_number = !sequence_number;
    else {
        printf("Packet rejected, sending again...\n");

        return llwrite(filedes, data, length);
    }

    return 0;
}

int llclose(int fd) {
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1) {
        perror("tcsetattr");
        return -1;
    }

    close(fd);

    return 0;
}

int llopen_receptor(int filedes) {
    printf("Awaiting connection establishment from receptor...\n");

    retries = 0;

    enum STATE state = START;

    char byte;

    while (state != STOP) {
        read(filedes, &byte, sizeof(byte));

        handle_state(&state, &byte, EMITTER_ADDRESS, CONTROL_SET);
    }

    if (state != STOP) {
        printf("Connection failed. No valid SET packet was received!\n");
        return -1;
    } else {
        printf("SET packet received.\n");
    }

    control_packet ua_packet = build_control_packet(EMITTER_ADDRESS, CONTROL_UA);
    write(filedes, &ua_packet, sizeof(ua_packet));

    return 0;
}

int llopen_emitter(int filedes) {
    printf("Establishing connection with receptor...\n");

    retries = 0;

    enum STATE state = START;

    control_packet packet = build_control_packet(EMITTER_ADDRESS, CONTROL_SET);

    do {
        write(filedes, &packet, sizeof(packet));

        alarm(3);

        char byte;

        flag = 0;

        while (!flag && state != STOP) {
            read(filedes, &byte, sizeof(byte));

            handle_state(&state, &byte, RECEPTOR_ADDRESS, CONTROL_UA);
        }
    } while (flag && retries <= MAX_TIMEOUT_RETRIES);

    if (state != STOP) {
        printf("Connection failed: timed out!\n");
        return -1;
    }

    return 0;
}