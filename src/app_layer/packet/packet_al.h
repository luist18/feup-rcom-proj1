#ifndef APP_PACKET_H
#define APP_PACKET_H

char* build_app_control_packet(char control_byte, char type[], unsigned char sizes[],
                               char** data, unsigned int size, unsigned int* packet_length);

char* build_app_data_packet(char sequence_number, unsigned int size, char* data, unsigned int* packet_length);

#endif