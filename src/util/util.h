#ifndef UTIL_H
#define UTIL_H

#include <time.h>

char *length_to_array(long file_length, unsigned char *size);

void convert_to_array(char *buffer, long file_length, unsigned int size);

long array_to_length(char *buffer, unsigned char size);

void print_progress_bar(long bytes_sent, long file_length, double start_time);

void print_packet(char *message, char *packet, long size);

void print_packet_information(int packet_size, int packet_number, long total_bytes);

double get_current_time();

#endif