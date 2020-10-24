#ifndef APP_LAYER_H
#define APP_LAYER_H

int al_send_packet(int fd, char* packet, unsigned int size);

int send_control_packet(int fd, char* filename, long file_length, char control_byte);

int send_data_packets(int fd, char* filename, long file_length);

int send_file(int fd, char* filename, long file_length);

#endif