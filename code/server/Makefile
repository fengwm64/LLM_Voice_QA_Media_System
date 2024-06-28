DIR_INC = ./include
DIR_BIN = ./bin
DIR_LIB = ./libs
DIR_OBJ = ./obj

TARGET = server_main
BIN_TARGET = $(DIR_BIN)/$(TARGET)

CROSS_COMPILE = 
CFLAGS = -g -Wall -I$(DIR_INC) -w

ifdef LINUX64
LDFLAGS := -L$(DIR_LIB)/x64
else
LDFLAGS := -L$(DIR_LIB)/x86 
endif
LDFLAGS += -lmsc -lrt -ldl -lpthread -lstdc++

SRC := $(wildcard *.c)
OBJECTS := $(patsubst %.c,$(DIR_OBJ)/%.o,$(SRC))

all: $(BIN_TARGET)

$(BIN_TARGET): $(OBJECTS)
	@mkdir -p $(DIR_BIN)
	$(CROSS_COMPILE)gcc $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(DIR_OBJ)/%.o: %.c
	@mkdir -p $(DIR_OBJ)
	$(CROSS_COMPILE)gcc -c $(CFLAGS) $< -o $@

clean:
	@rm -f $(DIR_OBJ)/*.o $(BIN_TARGET)

.PHONY: all clean
