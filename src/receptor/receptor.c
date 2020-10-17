#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>

#include "../data_link/packet/packet.h"
#include "../util/flags.h"

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP = FALSE;

void printPacketInformation(control_packet* const packet) {
    printf("Fields: \n");
    printf("I_Flag: 0x%02X\n", packet->I_FLAG);
    printf("Address: 0x%02X\n", packet->ADDRESS);
    printf("Control: 0x%02X\n", packet->CONTROL);
    printf("Protection Field: 0x%02X\n", packet->PROTECTION_FIELD);
    printf("F_Flag: 0x%02X\n", packet->F_FLAG);
    printf("Done!\n");
}

int main(int argc, char** argv) {
    int fd, c, res;
    struct termios oldtio, newtio;
    char buf[255];

    if ((argc < 2) ||
        ((strcmp("/dev/ttyS10", argv[1]) != 0) &&
         (strcmp("/dev/ttyS11", argv[1]) != 0))) {
        printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
        exit(1);
    }

    /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */

    fd = open(argv[1], O_RDWR | O_NOCTTY);
    if (fd < 0) {
        perror(argv[1]);
        exit(-1);
    }

    if (tcgetattr(fd, &oldtio) == -1) { /* save current port settings */
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

    /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) pr�ximo(s) caracter(es)
  */

    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd, TCSANOW, &newtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }

    printf("New termios structure set\n");

    /*=== GETTING SETUP MESSAGE! ===*/

    /*
      Wait until I get SET message
      [Flag, A, C, BCC, FLAG], 
      where FLAG is 0X7E, 
      A is 0x03 or 0x01 
      C is SET -> 0x03
      Bcc1 is XOR of field A and field C
      Flag is once again, 0x7E

      If bcc1 (locally calculated) is the same as the one that I received, then it's valid, otherwise ignore)
      When I receive a valid message, I must send UA back to the emitter
      [FLAG, A, UA, BCC1, FLAG].
    
    */

    control_packet packet;
    bzero(&packet, sizeof(packet));

    res = read(fd, &packet, sizeof(packet));

    //TODO Delete this later on, printing information just to check if all is good
    printPacketInformation(&packet);

    //TODO possibly put this on a util function bool checkIfPacketIsValidSetUp or something
    //Field Verification

    //Checking FLAG
    if (packet.I_FLAG != DELIMITER_FLAG) {
        printf("FLAG is different. Invalid packet.\n");
        exit(1);
    }
    if (packet.CONTROL != CONTROL_SET) {
        printf("Packet is not of set up. Terminating!\n");
        exit(2);
    }
    if (packet.PROTECTION_FIELD != (packet.ADDRESS ^ packet.CONTROL)) {
        printf("Protection field is invalid. Terminating!\n");
        exit(3);
    }
    printf("Packet is valid!\n");

    //At this point, packet is valid. Send UA to emitter to acknowledge he receival or set-up packet
    control_packet r_packet = build_control_packet(RECEPTOR_ADDRESS, CONTROL_UA);
    write(fd, &r_packet, sizeof(r_packet));
    printf("Sent UA to emitter!\n");

    /* 
    O ciclo WHILE deve ser alterado de modo a respeitar o indicado no gui�o 
  */

    tcsetattr(fd, TCSANOW, &oldtio);
    close(fd);
    return 0;
}
