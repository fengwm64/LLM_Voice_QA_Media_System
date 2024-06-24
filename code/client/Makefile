# 声明交叉编译器
CC = arm-linux-gcc
# 编译选项
CFLAGS = -I./lib/lcd -I./lib/bmp -I./lib/font -I./lib/jpg -I./lib/touch -I./lib/function
# 链接选项
LDFLAGS = -L./lib/jpg -ljpeg

# 源文件目录
SRC_DIR = ./
# 目标文件目录
OBJ_DIR = ./obj
# 可执行文件目录
BIN_DIR = ../build

# 所有源文件
SRCS = $(wildcard $(SRC_DIR)/*.c)
# 所有目标文件
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
# 目标可执行文件
TARGET = $(BIN_DIR)/client_main

# 子目录下的源文件 
SUB_DIRS = ./lib/lcd ./lib/bmp ./lib/font ./lib/jpg ./lib/touch ./lib/function
SUB_SRCS = $(foreach dir,$(SUB_DIRS),$(wildcard $(dir)/*.c))
SUB_OBJS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(notdir $(SUB_SRCS)))

# 默认目标
all: $(TARGET)

# 生成目标可执行文件
$(TARGET): $(OBJS) $(SUB_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $^ -o $@ $(LDFLAGS)

# 编译主目录下的源文件
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# 编译子目录下的源文件
$(OBJ_DIR)/%.o: ./lib/lcd/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: ./lib/bmp/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: ./lib/font/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: ./lib/jpg/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: ./lib/touch/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: ./lib/function/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# 清理目标文件和可执行文件
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
