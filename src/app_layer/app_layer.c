#include "app_layer.h"

#include "../data_link/data_link.h"

int al_send_packet(int fd, char* packet, unsigned int size) {
    return llwrite(fd, packet, size);
}