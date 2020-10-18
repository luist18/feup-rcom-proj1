#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>

#include "../data_link/packet/packet.h"
#include "../util/flags.h"

#include "../data_link/data_link.h"

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
    
    if ((argc < 2) ||
        ((strcmp("/dev/ttyS10", argv[1]) != 0) &&
         (strcmp("/dev/ttyS11", argv[1]) != 0))) {
        printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
        exit(1);
    }

    int fd;
    if (fd = llopen(argv[1], RECEPTOR), fd < 0)
        exit(1);

    if (llclose(fd) < 0)
        exit(2);


    return 0;
}
