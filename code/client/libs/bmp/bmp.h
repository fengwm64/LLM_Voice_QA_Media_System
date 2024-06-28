#ifndef __BMP_H_
#define __BMP_H_

// 头文件
#include "lcd.h"

//-------函数声明-----------

// 显示BMP图像到LCD屏幕上
int displayBMP(const char *bmp_path, int offset_x, int offset_y);

#endif