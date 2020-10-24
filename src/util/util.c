#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *length_to_array(long file_length, unsigned char *size) {
    long bl = file_length & 0xFFFFFFFF00000000;
    long bi = file_length & 0x00000000FFFF0000;
    long bs = file_length & 0x000000000000FF00;

    if (bl != 0) {
        char *buffer = malloc(8 * sizeof(char));

        convert_to_array(buffer, file_length, 8);
        *size = 8;

        return buffer;
    } else if (bi != 0) {
        char *buffer = malloc(4 * sizeof(char));

        convert_to_array(buffer, file_length, 4);
        *size = 4;

        return buffer;
    } else if (bs != 0) {
        char *buffer = malloc(2 * sizeof(char));

        convert_to_array(buffer, file_length, 2);
        *size = 2;

        return buffer;
    }

    char *buffer = malloc(1 * sizeof(char));

    convert_to_array(buffer, file_length, 1);
    *size = 1;

    return buffer;
}

void convert_to_array(char *buffer, long file_length, unsigned int size) {
    for (int i = 0; i < size; ++i)
        buffer[i] = (char)((file_length) >> (8 * i) & 0xFF);
}