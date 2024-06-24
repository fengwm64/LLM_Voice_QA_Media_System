DIR_INC = ./include
DIR_BIN = ../build
DIR_LIB = ./libs
DIR_OBJ = ./obj

TARGET = server_main
BIN_TARGET = $(DIR_BIN)/$(TARGET)

CROSS_COMPILE =
CFLAGS = -g -Wall -w -I$(DIR_INC)

ifdef LINUX64
LDFLAGS := -L$(DIR_LIB)/x64
else
LDFLAGS := -L$(DIR_LIB)/x86
endif
LDFLAGS += -lmsc -lrt -ldl -lpthread -lstdc++

# List of source files
SRCS := $(wildcard *.c)
# List of object files
OBJS := $(patsubst %.c,$(DIR_OBJ)/%.o,$(SRCS))

# Default target
all: $(BIN_TARGET)

# Rule to generate the executable
$(BIN_TARGET): $(OBJS) | $(DIR_BIN)
	$(CROSS_COMPILE)gcc $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Rule to compile C source files into object files
$(DIR_OBJ)/%.o: %.c | $(DIR_OBJ)
	$(CROSS_COMPILE)gcc -c $(CFLAGS) $< -o $@

# Ensure directories exist
$(DIR_OBJ):
	mkdir -p $(DIR_OBJ)

$(DIR_BIN):
	mkdir -p $(DIR_BIN)

# Clean rule
clean:
	rm -rf $(DIR_OBJ) *.o $(BIN_TARGET)

.PHONY: all clean
