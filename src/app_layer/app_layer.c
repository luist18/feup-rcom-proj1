#include "app_layer.h"

#include <string.h>

#include "../app_layer/packet/packet_al.h"
#include "../data_link/data_link.h"
#include "../util/flags.h"
#include "../util/util.h"

int al_send_packet(int fd, char *packet, unsigned int size)
{
    return llwrite(fd, packet, size);
}

int send_control_packet(int fd, char *filename, long file_length, char control_byte)
{
    unsigned char length_buffer_size;
    char *length_buffer = length_to_array(file_length, &length_buffer_size);

    int control_packet_length;
    char *control_packet = build_app_control_packet(control_byte, AL_TYPE_LENGTH, length_buffer_size, length_buffer, &control_packet_length);
    control_packet = append_app_control_packet(control_packet, AL_TYPE_NAME, strlen(filename) + 1, filename, control_packet_length, &control_packet_length);

    if (llwrite(fd, control_packet, control_packet_length) < 0)
    {
        printf("Could not send %s app packet!\n", control_byte == AL_CONTROL_START ? "start" : "start");
        return -1;
    }

    return 0;
}

int send_data_packets(int fd, char *filename, long file_length)
{
    FILE *file;

    file = fopen(filename, "rb");
    if (file == NULL)
    {
        printf("Error while opening file!\n");
        exit(6);
    }

    int bytes_sent = 0, bytes_read = 0, sequence_number = 0;

    char buffer[AL_PACKET_SIZE];

    while (bytes_sent != file_length)
    {
        int content_size = file_length - bytes_sent >= 0 ? AL_PACKET_SIZE : file_length - bytes_sent;

        bytes_read = fread(buffer, 1, content_size, file);

        unsigned int packet_length;
        char *packet = build_app_data_packet(sequence_number++, bytes_read, buffer, &packet_length);

        if (al_send_packet(fd, packet, packet_length) < 0)
            return -1;

        bytes_sent += content_size;
    }

    return 0;
}

int send_file(int fd, char *filename, long file_length)
{
    if (send_control_packet(fd, filename, file_length, AL_CONTROL_START) < 0)
        return -1;

    if (send_data_packets(fd, filename, file_length) < 0)
        return -1;

    if (send_control_packet(fd, filename, file_length, AL_CONTROL_END) < 0)
        return -1;

    return 0;
}

int read_control_packet(int fd, char *fileName, long *fileSize)
{
    char *filename;
    char *filesize;
    char *buffer;
    llread(fd, buffer);
    long valueFileSize;

    if (buffer[1] == 1)
    {
        int sizeOfFileName = buffer[2]; //number of fields that will be read
        filename = malloc(sizeOfFileName * sizeof(char));
        strncpy(filename, buffer + 2, sizeOfFileName); //copy the bytes starting at the 3rd position

        if (buffer[sizeOfFileName + 3] == 0)
        { //represents file size
            int sizeOfFileSize = buffer[sizeOfFileName + 4];
            filesize = malloc(sizeOfFileSize * sizeof(char));
            strncpy(filesize, buffer + sizeOfFileName + 5, sizeOfFileSize);
            valueFileSize = strtol(filesize, NULL, 10);
        }
        else
        {
            printf("Invalid field identifier!\n");
            return -1;
        }
    }
    else if (buffer[1] == 0)
    {

        int sizeOfFileSize = buffer[2];
        filesize = malloc(sizeOfFileSize * sizeof(char));
        strncpy(filesize, buffer + 2, sizeOfFileSize);
        valueFileSize = strtol(filesize, NULL, 10);

        if (buffer[sizeOfFileSize + 3] == 1)
        { //represents file size
            
            int sizeOfFileName = buffer[sizeOfFileSize + 4]; //number of fields that will be read
            filename = malloc(sizeOfFileName * sizeof(char));
            strncpy(filename, buffer + sizeOfFileSize + 5, sizeOfFileName); //copy the bytes starting at the 3rd position
        }
        else
        {
            printf("Invalid field identifier!\n");
            return -1;
        }
    }

    strncpy(fileName, filename, strlen(filename));
    memcpy(fileSize, &valueFileSize, sizeof(valueFileSize));

    return 0;
}

char* read_data_packet(int fd, unsigned int *data_length, int* sequence_number)
{
    char *buffer;
    
    llread(fd, buffer);

    unsigned int tmp_sequence_number = (unsigned int)buffer[1];
    unsigned int l2 = (unsigned int)buffer[2];
    unsigned int l1 = (unsigned int)buffer[3];

    *data_length = 256 * l2 + l1;
    *sequence_number = tmp_sequence_number;

    char *data = malloc(*data_length);
    memcpy(data, &buffer[4], *data_length);

    return data;
}

int read_data_packets(int fd, char *filename, long file_length)
{
    FILE *file;
    file = fopen(filename, "wb");

    int sequence_number = 0;
    long total_bytes_read = 0;
    unsigned int data_bytes_read;

    while (total_bytes_read != file_length)
    {
        int tmp_sequence_number;
        char *data = read_data_packet(fd, &data_bytes_read, &tmp_sequence_number);

        if (sequence_number != tmp_sequence_number)
        {
            fclose(file);
            return -1;
        }

        fwrite(data, data_bytes_read, 1, file);
        free(data);

        sequence_number++;
        sequence_number %= 255;

        total_bytes_read += data_bytes_read;
    }

    fclose(file);

    return 0;
}

int receive_file(int fd)
{
    char *filename;
    long file_length;

    if (read_control_packet(fd, filename, &file_length) < 0)
        return -1;

    if (read_data_packets(fd, filename, file_length) < 0)
        return -1;

    if (read_control_packet(fd, filename, &file_length) < 0)
        return -1;

    return 0;
}