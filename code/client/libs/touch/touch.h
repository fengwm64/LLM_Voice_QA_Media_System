#ifndef __TOUCH_H_
#define __TOUCH_H_

#include "../../includes.h"

// 设备文件
#define TOUCH_PATH "/dev/input/event0"

// 打开触摸屏
int init_touch();

// 关闭触控屏
void close_touch(int fd);

// 获取最后一次触摸的位置
void get_touch(int *input_x, int *input_y);

// 获取滑动方向
int get_direction();

// 等待用户点击
void wait_touch();

#endif