#include "touch.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>

#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4

// 宏定义
// 设备文件
#define TOUCH_PATH "/dev/input/event0"

int GetDirection()
{
    // 1. 打开触摸屏
    if (lcd_init() != 0)
    {
        printf("LCD 初始化失败\n");
        return 0;
    }

    int fd = open(TOUCH_PATH, O_RDONLY); // 只读方式打开设备文件
    if (fd < 0)
    {
        perror("打开触摸屏设备文件失败");
        lcd_close();
        return 0;
    }

    int x_start = -1, y_start = -1; // 初始坐标值
    int x_end = -1, y_end = -1;     // 终点坐标值

    struct input_event ev;
    while (1)
    {
        // 2. 不停地从设备文件中读取数据
        if (read(fd, &ev, sizeof(struct input_event)) < 0)
        {
            perror("读取触摸屏数据失败");
            close(fd);
            lcd_close();
            return 0;
        }

        // 3. 解析数据
        if (ev.type == EV_ABS)
        { // 触摸事件
            if (ev.code == ABS_X)
            {
                if (x_start == -1)
                {
                    x_start = ev.value; // 起点
                }
                x_end = ev.value; // 终点
            }
            else if (ev.code == ABS_Y)
            {
                if (y_start == -1)
                {
                    y_start = ev.value; // 起点
                }
                y_end = ev.value; // 终点
            }
        }
        else if (ev.type == EV_KEY && ev.code == BTN_TOUCH && ev.value == 0)
        { // 按键事件
            if (x_start != -1 && y_start != -1)
            {
                break;
            }
        }

        if (ev.type == EV_SYN)
        {
            if (x_start != -1 && y_start != -1)
            {
                // 根据触摸滑动的方向返回结果
                if (abs(x_end - x_start) > abs(y_end - y_start))
                {
                    if (x_end - x_start > 0)
                    {
                        close(fd);
                        lcd_close();
                        return RIGHT;
                    }
                    else
                    {
                        close(fd);
                        lcd_close();
                        return LEFT;
                    }
                }
                else
                {
                    if (y_end - y_start > 0)
                    {
                        close(fd);
                        lcd_close();
                        return DOWN;
                    }
                    else
                    {
                        close(fd);
                        lcd_close();
                        return UP;
                    }
                }
            }
        }
    }

    // 4. 关闭触摸屏
    close(fd);
    lcd_close();

    return 0;
}
