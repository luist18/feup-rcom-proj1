CC=gcc
BIN_DIR=bin
SRC_DIR=src

# Compiling extensions
COMPILATION_EXTENSIONS = -Wall -g -o
LINKED_LIBRARIES = -lm -lrt

# The names of the .c files to compile
COMMON_FILES = $(SRC_DIR)/data_link/packet/packet_dl.c $(SRC_DIR)/data_link/control/control.c $(SRC_DIR)/data_link/data_link.c $(SRC_DIR)/app_layer/packet/packet_al.c $(SRC_DIR)/app_layer/app_layer.c $(SRC_DIR)/util/flags.h $(SRC_DIR)/util/file.c $(SRC_DIR)/util/util.c
EMITTER_FILES = $(SRC_DIR)/emitter/emitter.c ${COMMON_FILES}
RECEPTOR_FILES = $(SRC_DIR)/receptor/receptor.c ${COMMON_FILES}

make: ${FILES}
	@mkdir -p $(BIN_DIR)
	@$(CC) ${COMPILATION_EXTENSIONS} $(BIN_DIR)/emitter ${EMITTER_FILES} ${LINKED_LIBRARIES}
	@$(CC) ${COMPILATION_EXTENSIONS}  $(BIN_DIR)/receptor ${RECEPTOR_FILES} ${LINKED_LIBRARIES}
	@rm -f ./emitter ./receptor