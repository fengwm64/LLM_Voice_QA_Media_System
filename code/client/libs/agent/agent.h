#ifndef AGENT_H
#define AGENT_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/input.h>
#include <pthread.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdbool.h>
#include <errno.h>

#include "touch.h"
#include "font.h"
#include "lcd.h"
#include "lcdjpg.h"
#include "bmp.h"

struct Agent
{
    int idx;           // 当前显示到第几张照片
    int MAX_IDX;       // agent形象照片数量
    const char **bmp_files; // agent形象
    int MAX_LINE;      // 一行多长
};

// 初始化Agent
void agent_Init(struct Agent *agent, const char **bmp_files, int bmp_num, const char *message, int line);

// 切换Agent显示图片
void agent_Update(struct Agent *agent);

// 显示信息
void agent_Speak(struct Agent *agent, const char *message);

// 语音控制
char* speak_to_Agent(const char * record_name);

#endif /* AGENT_H */