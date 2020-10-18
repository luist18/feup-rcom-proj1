CC=gcc
BIN_DIR=bin
SRC_DIR=src

# Compiling extensions
COMPILATION_EXTENSIONS = -Wall -g -o
LINKED_LIBRARIES = -lm -lrt

# The names of the .c files to compile
EMITTER_FILES = $(SRC_DIR)/emitter/emitter.c $(SRC_DIR)/data_link/packet/packet.c $(SRC_DIR)/data_link/control/control.c $(SRC_DIR)/data_link/data_link.c $(SRC_DIR)/util/flags.h
RECEPTOR_FILES = $(SRC_DIR)/receptor/receptor.c $(SRC_DIR)/data_link/packet/packet.c $(SRC_DIR)/data_link/control/control.c $(SRC_DIR)/data_link/data_link.c $(SRC_DIR)/util/flags.h 

make: ${FILES}
	@mkdir -p $(BIN_DIR)
	@$(CC) ${COMPILATION_EXTENSIONS} $(BIN_DIR)/emitter ${EMITTER_FILES} ${LINKED_LIBRARIES}
	@$(CC) ${COMPILATION_EXTENSIONS}  $(BIN_DIR)/receptor ${RECEPTOR_FILES} ${LINKED_LIBRARIES}
	@rm -f ./emitter ./receptor