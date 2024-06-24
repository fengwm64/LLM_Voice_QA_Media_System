#include "lcd.h"   // 包含 LCD 屏幕相关函数
#include "bmp.h"   // 包含 BMP 图片处理函数
#include "touch.h" // 包含触摸屏处理函数

// 相册函数
int PhotoAlbum(int photoNum, char *photoName[])
{
    int rs = 0; // 变量存储触摸方向
    int i = 0;  // 变量记录当前图片索引

    // 初始化 LCD 屏幕
    if (lcd_init() != 0)
    { // 检查初始化是否成功
        printf("LCD 初始化失败\n");
        return -1; // 如果失败，返回错误代码
    }

    // 无限循环处理触摸输入
    while (1)
    {
        // 获取触摸输入的方向
        rs = GetDirection();
        printf("%d\n", rs); // 打印触摸方向用于调试

        // 如果触摸方向是右 (1) 或下 (3)
        if (1 == rs || 3 == rs)
        {
            // 增加图片索引，如果超过图片数量则循环回到第一张
            i = (i + 1) % photoNum;
            // 显示当前索引的图片
            show_bmp(photoName[i], 0, 0);
        }
        // 如果触摸方向是左 (2) 或上 (4)
        else if (2 == rs || 4 == rs)
        {
            // 减少图片索引，如果小于0则循环回到最后一张
            i = (i - 1 + photoNum) % photoNum;
            // 显示当前索引的图片
            show_bmp(photoName[i], 0, 0);
        }
    }

    // 关闭 LCD 屏幕（在无限循环中无法到达，但保持良好习惯）
    lcd_close();

    return 0; // 返回 0 表示正常执行
}
