#ifndef DATA_LINK_H
#define DATA_LINK_H

#include <stdio.h>
#include <stdlib.h>

/**
 * @brief The open type of llopen.
 */
enum open_type {
    EMITTER,
    RECEPTOR
};

struct application_layer {
    int filedes;
    enum open_type type;
};

struct link_layer {
    char port[20];
    int baud_rate;
    unsigned int sequence_number;
    unsigned int timeout;
    unsigned int timeout_number;
};

/**
 * @brief Establishes a connection in a specific port.
 * 
 * @param port      the port
 * @param open_type the open type
 * @return int      the filedescriptor if success, a negative number otherwise
 */
int llopen(char *port, enum open_type open_type);

/**
 * @brief Establishes the emitter connection.
 * 
 * @param filedes   the filedescriptor of the port
 * @return int      0 if success, a negative number otherwise
 */
int llopen_emitter(int filedes);

/**
 * @brief Establishes the receptor connection.
 * 
 * @param filedes   the filedescriptor of the port
 * @return int      0 if success, a negative number otherwise
 */
int llopen_receptor(int filedes);

/**
 * @brief Writes data in the file descriptor.
 * 
 * @param fd        the filedescriptor
 * @param data      the data to send
 * @param length    the length of the data buffer
 * @return int      0 if success, a negative number otherwise
 */
int llwrite(int fd, char *data, int length);

/**
 * @brief Reads the data from the file descriptor.
 * 
 * @param fd        the file descriptor
 * @param buffer    array with the received data
 * @return int      length of the array (number of characters read), negative value otherwise
 * 
 */
int llread(int fd, char *buffer);

/**
 * @brief Closes a connection.
 * 
 * @param fd    the filedescriptor
 * @param open_type the open type
 * @return int  0 if success, a negative number otherwise
 */
int llclose(int fd, enum open_type open_type);

#endif