#ifndef __LCD_H_
#define __LCD_H_

//------头文件声明-------
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

// ========= 宏定义 =========
#define BLUE 0x0000FF
#define LCD_PATH "/dev/fb0"

// lcd设备结构体
struct LcdDevice
{
    int fd;   // 文件描述符
    void *mp; // 映射内存指针
};

// ========= 函数声明 =========

// 屏幕打开
struct LcdDevice *lcd_init();

// 关闭LCD设备
void lcd_close(struct LcdDevice *lcd);

// 在指定位置显示颜色
void display_point(struct LcdDevice *lcd, int x, int y, int color);

// LCD 测试
void lcd_test(struct LcdDevice *lcd, int color);

#endif