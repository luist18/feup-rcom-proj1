#include "packet.h"

#include <string.h>
#include <unistd.h>

#include "../../util/flags.h"

control_packet build_control_packet(char address, char control) {
    control_packet packet = {.I_FLAG = DELIMITER_FLAG,
                             .ADDRESS = address,
                             .CONTROL = control,
                             .PROTECTION_FIELD = address ^ control,
                             .F_FLAG = DELIMITER_FLAG};

    return packet;
}

unsigned char *build_information_packet(char *data, unsigned int length, unsigned int sequence_number) {
    unsigned int new_length;

    unsigned char *data_stuffed = stuff(data, length, &new_length);

    unsigned char *packet = malloc(INFORMATION_PACKET_BASE_SIZE + new_length);

    packet[0] = DELIMITER_FLAG;
    packet[1] = EMITTER_ADDRESS;
    packet[2] = sequence_number << 6;
    packet[3] = DELIMITER_FLAG ^ EMITTER_ADDRESS;
    memcpy(&packet[4], data_stuffed, new_length);
    packet[new_length + 4] = get_data_bcc(data, length);
    packet[new_length + 5] = DELIMITER_FLAG;

    return packet;
}

unsigned char *stuff(char *data, unsigned int length, unsigned int *new_length) {
    unsigned char *stuffed_data = malloc(length * sizeof(char));

    *new_length = 0;
    unsigned int offset = 0;

    for (int i = 0; i < length; ++i) {
        char byte = data[i];

        if (byte == DELIMITER_FLAG) {
            stuffed_data[*new_length] = ESCAPE;
            stuffed_data[(*new_length)++] = DELIMITER_FLAG ^ 0x20;

            offset++;

            stuffed_data = realloc(stuffed_data, length + offset);
        } else if (byte == ESCAPE) {
            stuffed_data[*new_length] = ESCAPE;
            stuffed_data[(*new_length)++] = ESCAPE ^ 0x20;

            offset++;

            stuffed_data = realloc(stuffed_data, length + offset);
        } else {
            stuffed_data[*new_length] = byte;
        }

        (*new_length)++;
    }

    return stuffed_data;
}

char *destuff(char *data, unsigned int length) {
    //TODO Must check if I must reallocate memory if actual destuffing occurs
    char *destuffed_data = malloc(length * sizeof(char));

    /*Field 0 ->Flag
    Field 1 ->Address
    Field 2 ->Control
    Field 3 ->BCC
    These states are not victim of (de)stuffing, only the "internal" data*/
    for (int i = 0; i <= 3; i++) {
        destuffed_data[i] = data[i];
    }

    //Destuffing the internal bytes
    char byte;
    int current_position = 4;
    for (int i = 4; i < length-2; i++, current_position++){
        byte = data[i];
        if (byte == ESCAPE ){
            if (data[i+1] == 0x5E){
                destuffed_data[current_position] = 0x7E;
                i++; //Skip over the byte next to the ESCAPE   
            }
            else if(data[i+1] == 0x5D){
                destuffed_data[current_position] = ESCAPE;
                i++;
            }
        }
        else{
        
            destuffed_data[current_position] = byte;
        }
    }

    //Adding the remaining elements to the final array
    destuffed_data[length - 2] = data[length - 2];  //BCC2
    destuffed_data[length - 1] = data[length - 1];  //Flag

    return destuffed_data;
}

unsigned char get_data_bcc(char *data, unsigned int length) {
    unsigned char bcc = data[0];

    for (int i = 1; i < length; ++i)
        bcc ^= data[i];

    return bcc;
}
