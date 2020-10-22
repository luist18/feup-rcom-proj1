#ifndef APP_PACKET_H
#define APP_PACKET_H

unsigned char* build_app_control_packet(unsigned char control_byte, unsigned char type[], unsigned char sizes[],
                                        char** data, unsigned int size, unsigned int* packet_length);

unsigned char* build_app_data_packet(unsigned char sequence_number, unsigned int size, unsigned char* data, unsigned int* packet_length);

#endif