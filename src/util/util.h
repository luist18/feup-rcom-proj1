#ifndef UTIL_H
#define UTIL_H

char *length_to_array(long file_length, unsigned char *size);

void convert_to_array(char *buffer, long file_length, unsigned int size);

long array_to_length(char *buffer, unsigned char size);

void print_progress_bar(long bytes_sent, long file_length);

void print_packet(char *message, char *packet, long size);

void print_packet_information(int packet_size, int packet_number, long total_bytes);

#endif