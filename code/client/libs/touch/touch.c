#include "touch.h"

#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4

// 打开触摸屏
int init_touch()
{
    int fd = open(TOUCH_PATH, O_RDONLY); // 只读方式打开设备文件
    if (fd < 0)
    {
        perror("打开触摸屏设备文件失败");
        return -1;
    }

    return fd;
}

// 关闭触控屏
void close_touch(int fd)
{
    close(fd);
}

// 获取最后一次触摸的位置
void get_touch(int *input_x, int *input_y)
{
    int fd = init_touch();

    struct input_event buf;
    int x, y;

    while (1)
    {
        read(fd, &buf, sizeof(buf));

        // 坐标事件
        if (buf.type == EV_ABS && buf.code == ABS_X)
        {
            x = buf.value;
            x = x * 800 / 1024; // 黑色框才会需要这行代码
        }
        if (buf.type == EV_ABS && buf.code == ABS_Y)
        {
            y = buf.value;
            y = y * 480 / 600; // 黑色框才会需要这行代码
        }
        if (buf.type == EV_KEY && buf.code == BTN_TOUCH && buf.value == 1)
        {
            // 当手指离开屏幕时,打印最后一次触摸的位置
            *input_x = x;
            *input_y = y;
            printf("点击:%d, %d\n", *input_x, *input_y);
            break;
        }
    }

    close_touch(fd);
}

// 获取滑动方向
int get_direction()
{
    // 1. 打开触摸屏
    int fd = init_touch();

    int x_start = -1, y_start = -1; // 初始坐标值
    int x_end = -1, y_end = -1;     // 终点坐标值

    struct input_event ev;
    while (1)
    {
        // 2. 不停地从设备文件中读取数据
        if (read(fd, &ev, sizeof(struct input_event)) < 0)
        {
            perror("读取触摸屏数据失败");
            close_touch(fd);
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
                        close_touch(fd);
                        return RIGHT;
                    }
                    else
                    {
                        close_touch(fd);
                        return LEFT;
                    }
                }
                else
                {
                    if (y_end - y_start > 0)
                    {
                        close_touch(fd);
                        return DOWN;
                    }
                    else
                    {
                        close_touch(fd);
                        return UP;
                    }
                }
            }
        }
    }

    // 4. 关闭触摸屏
    close_touch(fd);
    return 0;
}

// 等待用户点击
void wait_touch()
{
    int x, y;
    printf("Waiting for touch...\n");
    get_touch(&x, &y);
    return;
}