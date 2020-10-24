#ifndef APP_LAYER_H
#define APP_LAYER_H

int al_send_packet(int fd, char* packet, unsigned int size);

int send_control_packet(int fd, char* filename, long file_length, char control_byte);

int send_data_packets(int fd, char* filename, long file_length);

int send_file(int fd, char* filename, long file_length);

char* read_data_packet(int fd, unsigned int *data_length, int* sequence_number);

int read_control_packet(int fd, char* fileName, long *fileSize);

int receive_file(int fd);

#endif