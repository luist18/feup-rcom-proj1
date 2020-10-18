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

int retries = 3;
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
            //TODO finished comparing until here

            setup_handler();

            if (llopen_emitter(fd) < 0)
                return -1;

            printf("Connection established.\n");
            return_code = fd;
            break;

        case RECEPTOR:
            fd = open(port, O_RDWR | O_NOCTTY);
            if (fd < 0){
                perror(port);
                exit(-1);
            }

            if (tcgetattr(fd, &oldtio) == -1){ 
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

            if (tcsetattr(fd, TCSANOW, &newtio) == -1){
                perror("tcsetattr");
                exit(-1);
            }   

            if (llopen_receptor(fd) < 0 ){
                return -1;
            }
            return_code = fd;




            break;

        default:
            return_code = -1;
            break;
    }

    return return_code;
}

int llwrite(int filedes, char *data, int length) {
    // TODO: replace with a default number
    retries = 3;

    enum STATE state = START;

    unsigned char *packet = build_information_packet(data, length, sequence_number);

    do {
        write(filedes, &packet, sizeof(packet));

        alarm(3);

        char byte;

        flag = 0;

        while (!flag && state != STOP) {
            read(filedes, &byte, sizeof(byte));

            // handle state for RR e REJ
        }
    } while (flag && retries <= MAX_TIMEOUT_RETRIES);

    if (state != STOP) {
        printf("Connection failed: timed out!\n");
        return -1;
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

int llopen_receptor(int filedes){
    printf("Awaiting connection establishment from receptor...\n");
    
    // TODO: replace with a default number
    retries = 3;

    enum STATE state = START;

    

    do{
        char byte;
        flag = 0;

        while (!flag && state != STOP) {
            read(filedes, &byte, sizeof(byte));

            handle_state(&state, &byte, EMITTER_ADDRESS, CONTROL_SET);
        }

    } while (flag && retries <= MAX_TIMEOUT_RETRIES);


    if (state != STOP){
        printf("Connection failed. No vaild set up message was received!\n");
        return -1;
    }
    else{
        printf("Everything worked out perfectly. Going to write UA Message\n");
    }


    /*
        TODO: Check what happens if sending UA fails. Might need to redo this part
    */
    control_packet ua_packet = build_control_packet(RECEPTOR_ADDRESS, CONTROL_UA);
    write(filedes, &ua_packet, sizeof(ua_packet));
    
    return 0;

}



int llopen_emitter(int filedes) {
    printf("Establishing connection with receptor...\n");

    // TODO: replace with a default number
    retries = 3;

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