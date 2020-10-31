#ifndef FLAGS_H
#define FLAGS_H

#include <stdio.h>
#include <stdlib.h>

/**
 * Data link configuration.
 */

#define MAX_TIMEOUT_RETRIES 3

#define BAUDRATE 78000

#define DELIMITER_FLAG 0x7e
#define ESCAPE 0x7d

#define EMITTER_ADDRESS 0x03
#define RECEPTOR_ADDRESS 0x01

#define CONTROL_SET 0x03
#define CONTROL_DISC 0x0b
#define CONTROL_UA 0x07
#define CONTROL_RR(v) (((v) == 1 ? 0x80 : 0x00) | 0x05)
#define CONTROL_REJ(v) (((v) == 1 ? 0x80 : 0x00) | 0x01)

#define INFO_SEQUENCE(v) ((v) == 1 ? 0x40 : 0x00)

#define INFORMATION_PACKET_BASE_SIZE 6 * sizeof(char)

/**
 * END - Data link configuration.
 */

/**
 * END - App layer configuration.
 */

#define AL_CONTROL_DATA 0x01
#define AL_CONTROL_START 0x02
#define AL_CONTROL_END 0x03

#define AL_TYPE_LENGTH 0x00
#define AL_TYPE_NAME 0x01

#define AL_PACKET_SIZE 3000

#define AL_DATA_PACKET_BASE_SIZE 4 * sizeof(char)

#define PACKET_SIZE AL_PACKET_SIZE + AL_DATA_PACKET_BASE_SIZE + INFORMATION_PACKET_BASE_SIZE

/**
 * END - App layer configuration.
 */
#endif
