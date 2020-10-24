#include "file.h"

#include <stdio.h>
#include <stdlib.h>

long get_file_length(char *filename) {
    FILE *file;
    long length;

    file = fopen(filename, "rb");

    if (file == NULL)
        return -1;

    if (fseek(file, 0, SEEK_END) < 0)
        return -1;

    if ((length = ftell(file)) < 0)
        return -1;

    return length;
}