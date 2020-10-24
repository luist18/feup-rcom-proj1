#ifndef APP_PACKET_H
#define APP_PACKET_H

char* build_app_control_packet(char control_byte, unsigned char type, unsigned char size,
                               char* data, int* packet_length);

char* append_app_control_packet(char* buffer, unsigned char type, unsigned char size,
                                char* data, int old_length, int* packet_length);

char* build_app_data_packet(char sequence_number, unsigned int size, char* data, unsigned int* packet_length);

#endif