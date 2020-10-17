#ifndef FLAGS_H
#define FLAGS_H

#include <stdio.h>
#include <stdlib.h>

/**
 * Data link configuration.
 */

#define MAX_TIMEOUT_RETRIES 3

#define BAUDRATE B38400

#define DELIMITER_FLAG 0x7e
#define ESCAPE 0x7d

#define EMITTER_ADDRESS 0x03
#define RECEPTOR_ADDRESS 0x01

#define CONTROL_SET 0x03
#define CONTROL_DISC 0x0b
#define CONTROL_UA 0x07
#define CONTROL_RR(v) (((v) == 1 ? 0x80 : 0x00) | 0x05)
#define CONTROL_REJ(v) (((v) == 1 ? 0x80 : 0x00) | 0x01)

#define INFORMATION_PACKET_BASE_SIZE 6 * sizeof(char)

/**
 * END - Data link configuration.
 */
#endif