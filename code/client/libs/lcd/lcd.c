#include "lcd.h"

// 初始化LCD设备
struct LcdDevice *lcd_init()
{
    struct LcdDevice *lcd = malloc(sizeof(struct LcdDevice));
    if (lcd == NULL)
    {
        fprintf(stderr, "分配内存失败: %s\n", strerror(errno));
        return NULL;
    }

    lcd->fd = open(LCD_PATH, O_RDWR);
    if (lcd->fd < 0)
    {
        perror("打开LCD设备失败");
        free(lcd);
        return NULL;
    }

    lcd->mp = mmap(NULL, 800 * 480 * 4, PROT_READ | PROT_WRITE, MAP_SHARED, lcd->fd, 0);
    if (lcd->mp == MAP_FAILED)
    {
        perror("映射LCD设备失败");
        close(lcd->fd);
        free(lcd);
        return NULL;
    }

    return lcd;
}

// 关闭LCD设备
void lcd_close(struct LcdDevice *lcd)
{
    if (lcd != NULL)
    {
        if (lcd->mp != MAP_FAILED)
        {
            if (munmap(lcd->mp, 800 * 480 * 4) == -1)
            {
                perror("取消映射内存失败");
            }
        }

        if (lcd->fd >= 0)
        {
            if (close(lcd->fd) == -1)
            {
                perror("关闭文件描述符失败");
            }
        }

        free(lcd);
    }
}

// 在指定位置显示颜色
void display_point(struct LcdDevice *lcd, int x, int y, int color)
{
    if (lcd == NULL || lcd->mp == MAP_FAILED)
    {
        fprintf(stderr, "LCD 未初始化或映射失败\n");
        return;
    }

    // 计算像素位置
    int *fb = (int *)lcd->mp;
    fb[y * 800 + x] = color;
}

// LCD 测试
void lcd_test(struct LcdDevice *lcd, int color)
{
    if (lcd == NULL)
    {
        fprintf(stderr, "LCD设备不存在\n");
        return;
    }

    int x, y;
    for (y = 0; y < 480; y++) // 遍历每一行
    {
        for (x = 0; x < 800; x++) // 遍历每一列
        {
            display_point(lcd, x, y, color);
        }
    }
}