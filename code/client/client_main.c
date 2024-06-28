#include "includes.h"

// photo与music相关
// ======================================================= //
#define NUM_OF_MUSIC 3
#define NUM_OF_VIDEO 2

// input_x, input_y: 触摸到的坐标
int input_x, input_y;

int main()
{
    // 显示启动界面
    displayBMP("start.bmp", 0, 0);

    // 等待1s
    sleep(2);

    // 显示主界面
    displayBMP("main.bmp", 0, 0);
    // sleep(5);

    // ai_Agent();

    // 等待用户触摸
    while (1)
    {
        while (1)
        {
            // 显示菜单界面，等待触摸输入
            get_input("main");

            int input_x_max = 800;
            // 计算每个区间的宽度
            int interval_width = input_x_max / 4;

            if (input_y <= 240)
            {
                if (input_x >= 0 && input_x < interval_width)
                {
                    printf("点击photo\n");
                    photo_play();
                }
                else if (input_x >= interval_width && input_x < interval_width * 2)
                {
                    printf("点击ai\n");
                    ai_Agent();
                    get_input("main");
                }
                else if (input_x >= interval_width * 2 && input_x < interval_width * 3)
                {
                    printf("点击music\n");
                    music_play();
                }
                else if (input_x >= interval_width * 3 && input_x <= input_x_max)
                {
                    printf("点击movic\n");
                    video_play();
                    printf("出来了");
                }
                else
                {
                    // 点击了空白处，重新等待输入
                    get_input("main");
                }
            }
            else
            {
                get_input("main");
            }
        }
        return 0;
    }

    return 0;
}
