#include "data_link.h"

#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>

#include "../util/flags.h"
#include "../util/util.h"
#include "control/control.h"
#include "packet/packet_dl.h"

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

    unsigned int byte_count = 0;

    char byte;

    char read_buffer[PACKET_SIZE];

    while (state != INFO_STOP) {
        read(fd, &byte, sizeof(byte));

        if (byte == ESCAPE) {
            char next_byte;
            read(fd, &next_byte, sizeof(byte));

            if (next_byte == (DELIMITER_FLAG ^ 0x20)) {
                read_buffer[byte_count++] = DELIMITER_FLAG;
            } else if (next_byte == (ESCAPE ^ 0x20)) {
                read_buffer[byte_count++] = ESCAPE;
            }
        } else {
            read_buffer[byte_count++] = byte;
        }

        handle_state_receptor_information(&state, &byte, EMITTER_ADDRESS, INFO_SEQUENCE(sequence_number));
    }

    char data_bcc2 = read_buffer[byte_count - 2];

    char bcc2 = read_buffer[4];

    for (int i = 5; i < byte_count - 2; ++i)
        bcc2 ^= read_buffer[i];

    int data_sequence_number = read_buffer[2] >> 6;

    if (data_sequence_number == sequence_number && data_bcc2 == bcc2) {
        control_packet rr_packet = build_control_packet(EMITTER_ADDRESS, CONTROL_RR(sequence_number));

        write(fd, &rr_packet, sizeof(rr_packet));

        sequence_number = !sequence_number;
    } else {
        control_packet rej_packet = build_control_packet(EMITTER_ADDRESS, CONTROL_REJ(sequence_number));

        print_packet("Packet rejected: ", read_buffer, byte_count);

        write(fd, &rej_packet, sizeof(rej_packet));

        return llread(fd, buffer);
    }

    for (int i = 4; i < byte_count - 1; ++i)
        buffer[i - 4] = read_buffer[i];

    return byte_count - INFORMATION_PACKET_BASE_SIZE;
}

int llwrite(int filedes, char *data, int length) {
    retries = 0;

    enum STATE state = START;
    enum STATE control;

    unsigned int packet_length;
    unsigned char *packet = build_information_packet(data, length, sequence_number, &packet_length);

    do {
        write(filedes, &(*packet), packet_length);

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

    alarm(0);

    free(packet);

    if (state != STOP) {
        printf("Connection failed: timed out!\n");
        return -1;
    }

    if (control == RR_RCV) {
        sequence_number = !sequence_number;
    } else {
        printf("Packet rejected, sending again...\n");

        return llwrite(filedes, data, length);
    }

    return packet_length;
}

int llclose(int fd, enum open_type open_type) {
    enum STATE state = START;

    switch (open_type) {
        case EMITTER:
            retries = 0;

            control_packet packet = build_control_packet(EMITTER_ADDRESS, CONTROL_DISC);

            do {
                write(fd, &packet, sizeof(packet));
                printf("Terminating connection with receptor...\n");

                alarm(3);

                char byte;

                flag = 0;

                while (!flag && state != STOP) {
                    read(fd, &byte, sizeof(byte));

                    handle_state(&state, &byte, EMITTER_ADDRESS, CONTROL_DISC);
                }
            } while (flag && retries <= MAX_TIMEOUT_RETRIES);

            alarm(0);

            if (state != STOP) {
                printf("Connection failed: timed out!\n");
                return -1;
            }

            packet = build_control_packet(EMITTER_ADDRESS, CONTROL_UA);

            write(fd, &packet, sizeof(packet));

            printf("Successfully disconnected.\n");

            break;
        case RECEPTOR:
            printf("Awayting termination by the emitter...\n");

            char byte;

            state = START;

            while (state != STOP) {
                read(fd, &byte, sizeof(byte));

                handle_state(&state, &byte, EMITTER_ADDRESS, CONTROL_DISC);
            }

            if (state != STOP) {
                printf("Failed to disconnect. No valid DISC packet was received!\n");
                return llclose(fd, open_type);
            }

            packet = build_control_packet(EMITTER_ADDRESS, CONTROL_DISC);

            write(fd, &packet, sizeof(packet));

            state = START;

            while (state != STOP) {
                read(fd, &byte, sizeof(byte));

                handle_state(&state, &byte, EMITTER_ADDRESS, CONTROL_UA);
            }

            if (state != STOP) {
                printf("Failed to disconnect. No valid UA packet was received!\n");
                return -1;
            }

            printf("Successfully disconnected.\n");

            break;
        default:
            return -1;
    }

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
    }

    printf("Connection established.\n");

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

            handle_state(&state, &byte, EMITTER_ADDRESS, CONTROL_UA);
        }
    } while (flag && retries <= MAX_TIMEOUT_RETRIES);

    alarm(0);

    if (state != STOP) {
        printf("Connection failed: timed out!\n");
        return -1;
    }

    return 0;
}