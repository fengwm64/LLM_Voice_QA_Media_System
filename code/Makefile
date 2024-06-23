# 定义编译器和编译选项的变量
ARM_CC = arm-linux-gcc
CC = gcc
CFLAGS = -L./ -lfont -lm

# 输出目录
BIN_DIR = ../bin

# 目标文件和源文件
CLIENT_SRC = tcp_client.c
SERVER_SRC = tcp_server.c
CLIENT_BIN = $(BIN_DIR)/client
SERVER_BIN = $(BIN_DIR)/server

# 默认目标
default: build

# 编译目标
build: $(CLIENT_BIN) $(SERVER_BIN)

# 为ARM平台编译客户端程序
$(CLIENT_BIN): $(CLIENT_SRC)
	mkdir -p $(BIN_DIR)  # 确保输出目录存在
	$(ARM_CC) $(CLIENT_SRC) -o $(CLIENT_BIN) $(CFLAGS)

# 为本地平台编译服务器程序
$(SERVER_BIN): $(SERVER_SRC)
	mkdir -p $(BIN_DIR)  # 确保输出目录存在
	$(CC) $(SERVER_SRC) -o $(SERVER_BIN)

# 清理目标
clean:
	rm -rf *.o
	rm -rf $(CLIENT_BIN)
	rm -rf $(SERVER_BIN)

# 运行目标
run: build
	$(SERVER_BIN)
