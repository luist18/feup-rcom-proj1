#include <stdio.h>
#include <stdlib.h>

#define BAUDRATE B38400

enum open_type {
    EMITTER,
    RECEPTOR
};

struct application_layer {
    int filedes;
    enum open_type type;
};

struct link_layer {
    char port[20];
    int baud_rate;
    unsigned int sequence_number;
    unsigned int timeout;
    unsigned int timeout_number;
};

int llopen(char *port, enum open_type open_type);

int llclose(int fd);