CC=gcc
BIN_DIR=bin
SRC_DIR=src

# Compiling extensions
COMPILATION_EXTENSIONS = -Wall -g -o
LINKED_LIBRARIES = -lm -lrt

# The names of the .c files to compile
EMITER_FILES = $(SRC_DIR)/emiter/emiter.c 
RECEPTOR_FILES = $(SRC_DIR)/receptor/receptor.c 

make: ${FILES}
	@mkdir -p $(BIN_DIR)
	@$(CC) ${COMPILATION_EXTENSIONS} $(BIN_DIR)/emiter ${EMITER_FILES} ${LINKED_LIBRARIES}
	@$(CC) ${COMPILATION_EXTENSIONS}  $(BIN_DIR)/receptor ${RECEPTOR_FILES} ${LINKED_LIBRARIES}
	@rm -f ./emiter ./receptor