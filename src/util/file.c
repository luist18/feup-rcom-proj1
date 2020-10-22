#include "file.h"

#include <stdio.h>
#include <stdlib.h>

long read_file(char *filename, char *buffer) {
    FILE *file;
    long length;

    file = fopen(filename, "rb");

    if (file == NULL)
        return -1;

    if (fseek(file, 0, SEEK_END) < 0)
        return -1;

    if ((length = ftell(file)) < 0)
        return -1;

    rewind(file);

    buffer = (char *)malloc(length * sizeof(char));

    if (fread(buffer, length, 1, file) < 0)
        return -1;

    if (fclose(file) < 0)
        return -1;

    return length;
}