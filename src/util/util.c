#include "util.h"

#include <math.h>
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
        buffer[size - 1 - i] = (char)((file_length) >> (8 * i) & 0xFF);
}

long array_to_length(char *buffer, unsigned char size) {
    long result = 0;

    for (int i = 0; i < size; ++i) {
        unsigned char byte = buffer[i];

        result |= (byte << 8 * (size - 1 - i));
    }

    return result;
}

const char *progress_bar_chars[] = {
    "▏",
    "▎",
    "▌",
    "▍",
    "▋",
    "▊",
    "▉",
    "█"};

void print_progress_bar(long bytes_sent, long file_length) {
    float progress = bytes_sent * 1.0f / file_length;

    int bar_number = progress / (2.5f / 100.0f);
    int frac_bar_number = (int)(fmod(progress, (2.5f / 100.0f)) / (0.357143 / 100.0f));

    printf("[");

    for (int i = 0; i < bar_number; ++i)
        printf("%s", progress_bar_chars[7]);

    printf("%s", progress_bar_chars[frac_bar_number]);

    for (int i = 0; i < 40 - bar_number; ++i)
        printf(" ");

    printf("] %6.2f %% ", progress * 100.0f);

    fflush(stdout);

    if (bar_number == 40)
        printf("\n");
    else
        printf("\r\033[K");
}