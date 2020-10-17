#include "packet.h"

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