#include "function.h"

// Agent相关
// ======================================================= //
#define BUFFER_SIZE 1024
#define LINE 10
#define MAX_IMAGES 3
#define INIT_MSG "我是你的智能语音助手，有什么事情请随时问我吧！"

const char *agent_pics[MAX_IMAGES] = {
    "a1.bmp",
    "a2.bmp",
    "a3.bmp"};

// 录音文件名
#define RECORD_CMD "arecord -d5 -c1 -r16000 -twav -fS16_LE"
#define RECORD_NAME "cmd.wav"
// ======================================================= //

// photo与music相关
// ======================================================= //
#define NUM_OF_MUSIC 3
#define NUM_OF_VIDEO 2

// input_x, input_y: 触摸到的坐标
int input_x, input_y;

// 用于指示是否检测到触摸输入的变量
int photo_touch_detected = 0;

int video_touch_detected = 0;

int stop_thread_videotouch = 1;

int video_is_playing = 0;

int number_song = 1;  // number_song的值为多少,就代表当前播放的是第几首歌
int number_video = 1; // number_video的值为多少,就代表当前播放的是第几首歌
int flag = 1;         //// 全局变量用于控制线程暂停

int score_10 = 0;
int score_01 = 0;
// ======================================================= //



/******************************************Agent*******************************************************/
void ai_Agent()
{
    struct Agent agent;

    // 初始化Agent
    agent_Init(&agent, agent_pics, MAX_IMAGES, INIT_MSG, LINE);

    // 等待用户点击屏幕
    wait_touch();
    agent_Update(&agent);

    // 录音
    printf("开始录音\n");
    char command[256];
    sprintf(command, "%s %s", RECORD_CMD, RECORD_NAME);
    printf("Command: %s\n", command);
    system(command);
    agent_Update(&agent);

    // 发送至服务器进行命令解析
    char *res = speak_to_Agent(RECORD_NAME);

    printf("\n%s\n", res);

    // 显示回答
    agent_Speak(&agent, res);

     // 处理响应内容
    if (strstr(res, "photo") != NULL)
    {
        photo_play();
        printf("正在打开相册\n");
    }
    else if (strstr(res, "music") != NULL)
    {
        music_play();
        printf("正在打开音乐\n");
    }
    else if (strstr(res, "video") != NULL)
    {
        video_play();
        printf("正在打开视频\n");
    }
    else
    {
        sleep(2);
    }

    free(res);
}


/******************************************相册*******************************************************/
void photo_play()
{
    // 显示菜单界面，等待触摸输入
    get_input("p");
    // 手动模式120*60  (100,280)
    while (1)
    {
        if (input_x >= 575 && input_x <= 774 && input_y >= 393 && input_y <= 451)
        {
            hand(); // 执行手势识别代码
            show_bmp("p.bmp");
        }
        if (input_x >= 573 && input_x <= 774 && input_y >= 82 && input_y <= 155)
        {
            manual(); // 执行手动代码
            show_bmp("p.bmp");
        }
        else if (input_x >= 573 && input_x <= 774 && input_y >= 188 && input_y <= 250)
        {
            // 执行自动代码
            // autom();
            pthread_t thread_autom, thread_touch;

            // 创建播放图片的线程
            pthread_create(&thread_autom, NULL, autom, NULL);

            // 创建检测触摸输入的线程
            pthread_create(&thread_touch, NULL, touch_detection, NULL);

            // 等待两个线程结束
            pthread_join(thread_touch, NULL);
            pthread_join(thread_autom, NULL);

            get_input("p");
        }
        else if (input_x >= 573 && input_x <= 774 && input_y >= 300 && input_y <= 390)
        {
            // 退出
            return;
        }
        else
            // 点击了空白处，重新等待输入
            get_input("p");
    }
}

// 手动函数
void manual()
{
    show_bmp("1.bmp");
    int hand_x = 0;
    int i = 1; // i的值为多少,就代表当前显示的图片是第几张
    while (1)
    {
        touch_fun();
        if (input_x < 300) // 左边
        {
            i--;
            if (i == 0)
            {
                i = 4;
            }
            switch (i)
            {
            case 1:
                show_bmp("1.bmp");
                break;
            case 2:
                show_bmp("2.bmp");
                break;
            case 3:
                show_bmp("3.bmp");
                break;
            case 4:
                show_bmp("4.bmp");
                break;
            }
        }

        if (input_x >= 300 && input_x <= 500) // 中间
        {
            printf("退出大图\n");
            break;
        }

        if (input_x > 500) // 右边
        {
            i++;
            if (i == 5)
            {
                i = 1;
            }
            switch (i)
            {
            case 1:
                show_bmp("1.bmp");
                break;
            case 2:
                show_bmp("2.bmp");
                break;
            case 3:
                show_bmp("3.bmp");
                break;
            case 4:
                show_bmp("4.bmp");
                break;
            }
        }
    }
}

// 自动函数
void *autom(void *arg)
{
    // 包含图片文件名的数组
    char *image_files[] = {"1.bmp", "2.bmp", "3.bmp", "4.bmp"};
    int num_images = sizeof(image_files) / sizeof(image_files[0]);

    // 用于跟踪当前图片的索引
    int current_image_index = 0;
    for (; current_image_index < num_images; current_image_index++)
    {
        // 如果检测到触摸输入，则退出循环
        if (photo_touch_detected)
        {
            printf("检测到触摸，正在退出\n");
            photo_touch_detected = 0;
            pthread_exit(NULL);
        }

        // 显示当前图片
        show_bmp(image_files[current_image_index]);

        // 在显示下一张图片之前延迟一定时间（例如，2秒）
        sleep(1);
    }

    pthread_exit(NULL);
}

// 图片触摸检测函数
void *touch_detection(void *arg)
{
    touch_fun();
    photo_touch_detected = 1;
    pthread_exit(NULL);
}

// 手势识别
int hand()
{
    show_bmp("1.bmp");
    // 打开手势识别模块
    int head_fd = open("/dev/IIC_drv", O_RDWR);
    if (head_fd < 0)
    {
        perror("open IIC_drv");
        return -3;
    }
    char tmp;
    // 包含图片文件名的数组
    char *image_files[] = {"1.bmp", "2.bmp", "3.bmp", "4.bmp"};
    int num_images = sizeof(image_files) / sizeof(image_files[0]);

    // 用于跟踪当前图片的索引
    int k = 0;
    while (1)
    {
        read(head_fd, &tmp, 1);
        switch (tmp)
        {
        case 3:
            printf("左\n");
            k--;
            if (k < 0)
                k = 3;
            show_bmp(image_files[k]);
            sleep(0.5);
            break;
        case 4:
            printf("右\n");
            k++;
            if (k > 3)
                k = 0;
            show_bmp(image_files[k]);
            sleep(0.5);
            break;
        case 6:
            printf("远离\n");
            close(head_fd);
            return 0;
            break;
        }
    }
    // 3. 关闭文件
    close(head_fd);
    return 0;
}





// 显示菜单界面，等待触摸输入，需要显示哪张照片就输入名字
void get_input(char *name)
{
    // 构造图片文件名
    char filename[100]; // 假设文件名不超过 100 个字符
    snprintf(filename, sizeof(filename), "%s.bmp", name);

    printf("打开图片：%s\n", filename);

    // 显示图片
    show_bmp(filename);
    // 调用触摸功能
    touch_fun();
}

/******************************************音乐*******************************************************/
// 音乐切换程序
void music_play()
{
    while (1)
    {
        // 显示菜单界面，等待触摸输入
        get_input("music");

        int input_x_max = 800;
        // 计算每个区间的宽度
        int interval_width = input_x_max / 6;

        if (input_y >= 360)
        {
            if (input_x >= 0 && input_x < interval_width)
            {
                system("killall -9 madplay");
                printf("播放\n");
                music_number();
            }
            else if (input_x >= interval_width && input_x < interval_width * 2)
            {
                // 执行第二个操作
                // 上一首歌
                printf("上一首歌\n");
                system("killall -9 madplay");
                switch_music(0);
                music_number();
                printf("现在是第%d首歌\n", number_song);
            }
            else if (input_x >= interval_width * 2 && input_x < interval_width * 3)
            {
                // 执行第三个操作
                printf("继续\n");
                system("killall -18 madplay");
            }
            else if (input_x >= interval_width * 3 && input_x < interval_width * 4)
            {
                printf("暂停\n");
                system("killall -19 madplay");
            }
            else if (input_x >= interval_width * 4 && input_x < interval_width * 5)
            {
                // 执行第四个操作
                // 下一首歌
                printf("下一首歌\n");
                system("killall -9 madplay");
                switch_music(1);
                music_number();
                printf("现在是第%d首歌\n", number_song);
            }
            else if (input_x >= interval_width * 5 && input_x <= input_x_max)
            {
                // 执行第五个操作
                printf("结束播放\n");
                system("killall -9 madplay");
            }
            else
            {
                // 点击了空白处，重新等待输入
                get_input("music");
            }
        }
        else if (input_y <= 120)
        {
            if (input_x >= interval_width * 5 && input_x <= input_x_max)
            {
                // 返回主页面
                system("killall -9 madplay");
                return;
            }
        }
        else
        {
            get_input("music");
        }
    }
}

void switch_music(int flag)
{
    // flag = 0 上一首歌；1 下一首歌
    switch (flag)
    {
    case 0: // 切换上一首歌
        number_song--;
        if (number_song <= 0)
        {
            number_song = NUM_OF_MUSIC;
        }
        break;
    case 1:
        number_song++;
        if (number_song > NUM_OF_MUSIC)
        {
            number_song = 1;
        }
        break;
    default:
        printf("完蛋了 没有这个情况\n");
        break;
    }
}

// 对应数字的音乐播放程序
void music_number()
{
    switch (number_song)
    {
    case 1:
        system("madplay 1.mp3 -r&");
        break;
    case 2:
        system("madplay 2.mp3 -r&");
        break;
    case 3:
        system("madplay 3.mp3 -r&");
        break;
    }
}

/******************************************视频*******************************************************/
int video_play()
{
    while (1)
    {
        // 显示菜单界面，等待触摸输入
        get_input("music");

        // 计算每个区间的宽度
        int input_x_max = 800;
        int interval_width = input_x_max / 6;

        if (input_y >= 360)
        {
            if (input_x >= 0 && input_x < input_x_max)
            {
                // 判断管道文件movie_fifo是否存在，不存在则创建
                if (access("./movie_fifo", F_OK) != 0)
                {
                    printf("fifo  not exits\n");
                    // 创建管道文件
                    if (mkfifo("./movie_fifo", 777) == -1)
                    {
                        printf("make fifo error\n");
                        return -1;
                    }
                }
                int fifo_fd = open("./movie_fifo", O_RDWR); // 以读写权限打开管道文件movie_fifo
                if (fifo_fd == -1)
                {
                    printf("open fifo error\n");
                    return -2;
                }

                if (video_is_playing = 1)
                {
                    system("killall -9 mplayer");
                    printf("杀视频\n");
                    usleep(100);
                    video_is_playing = 0;
                }

                stop_thread_videotouch = 1;

                pthread_t thread_play, thread_videotouch;

                // 创建播放的线程
                pthread_create(&thread_play, NULL, video_number, NULL);

                // 创建检测触摸输入的线程
                pthread_create(&thread_videotouch, NULL, video_touch_detection, NULL);

                // 等待两个线程结束
                pthread_join(thread_videotouch, NULL);
                pthread_join(thread_play, NULL);
                break;
            }
            else
                get_input("music");
        }
        else if (input_y <= 120)
        {
            if (input_x >= interval_width * 5)
            {
                // 返回主页面
                if (video_is_playing = 1)
                {
                    system("killall -9 mplayer");
                    video_is_playing = 0;
                }
                printf("返回主界面");
            }
        }
        else
        {
            get_input("music");
        }
    }

    return 0;
}
// 视频切换程序

void switch_video(int flag)
{
    // flag = 0 上一个；1 下一个
    switch (flag)
    {
    case 0: // 切换上一个
        number_video--;
        if (number_video <= 0)
        {
            number_video = NUM_OF_VIDEO;
        }
        break;
    case 1:
        number_video++;
        if (number_video > NUM_OF_VIDEO)
        {
            number_video = 1;
        }
        break;
    default:
        printf("完蛋了 没有这个情况\n");
        break;
    }
}

// 对应数字的视频播放
void video_number_play()
{
    switch (number_video)
    {
    case 1:
        system("mplayer -quiet -geometry 0:0 -zoom -x 650 -y 300 -input file=./movie_fifo av1.avi &");
        video_is_playing = 1;
        break;
    case 2:
        system("mplayer -quiet -geometry 0:0 -zoom -x 650 -y 300 -input file=./movie_fifo av2.avi &");
        video_is_playing = 1;
        break;
    }
}

// 对应数字的视频播放按钮逻辑
void* video_number()
{
    int fifo_fd = open("./movie_fifo", O_RDWR); // 以读写权限打开管道文件movie_fifo
    printf("--------------------------\n");
    while (1)
    {
        if (video_touch_detected == 1)
        {
            if (video_is_playing == 1)
            {
                system("killall -9 mplayer");
                printf("杀视频\n");
                usleep(100);
                video_is_playing = 0;
            }
            video_number_play(1);
            printf("播放视频\n");
            video_touch_detected = 0;
        }
        else if (video_touch_detected == 2)
        {
            if (video_is_playing == 1)
            {
                system("killall -9 mplayer");
                printf("杀视频\n");
                usleep(100);
                video_is_playing = 0;
            }
            printf("上一个视频\n");
            switch_video(0);
            video_number_play(number_video);
            video_touch_detected = 0;
        }
        else if (video_touch_detected == 3)
        {
            // 继续播放
            printf("继续视频\n");
            write(fifo_fd, "pause\n", 6);
            video_touch_detected = 0;
        }
        else if (video_touch_detected == 4)
        {
            // 暂停播放
            printf("暂停视频\n");
            write(fifo_fd, "pause\n", 6);
            video_touch_detected = 0;
        }
        else if (video_touch_detected == 5)
        {
            // 继续播放
            if (video_is_playing == 1)
            {
                system("killall -9 mplayer");
                printf("杀视频\n");
                usleep(100);
                video_is_playing = 0;
            }
            printf("下一个视频\n");
            switch_video(1);
            video_number_play(number_video);
            video_touch_detected = 0;
        }
        else if (video_touch_detected == 6)
        {
            if (video_is_playing = 1)
            {
                system("killall -9 mplayer");
                printf("杀视频\n");
                usleep(100);
                video_is_playing = 0;
            }
            printf("结束播放");
            // video_touch_detected = 0;
            if (input_y <= 120)
            {
                if (input_x >= 734 && input_x <= 792 && input_y >= 8 && input_y <= 71)
                {
                    video_touch_detected = 0;

                    break;
                }
            }
        }
        else if (video_touch_detected == 7)
        {
            // 返回主页面
            if (video_is_playing = 1)
            {
                system("killall -9 mplayer");
                printf("杀视频\n");
                usleep(100);
                video_is_playing = 0;
            }
            printf("返回主界面");
            return 0;
        }
    }

    // stop_thread_videotouch = 0; // 停止检查触摸的线程
}

// 触摸检测函数
void* video_touch_detection(void* arg)
{
    while (stop_thread_videotouch)
    {
        get_input("music");

        int input_x_max = 800;
        // 计算每个区间的宽度
        int interval_width = input_x_max / 6;

        if (input_y >= 360)
        {
            if (input_x >= interval_width * 0 && input_x < interval_width * 1)
            {
                printf("播放 video_touch_detected = 1\n");
                video_touch_detected = 1; // 编号
            }
            else if (input_x >= interval_width * 1 && input_x < interval_width * 2)
            {
                // 上一个视频
                printf("上一个视频 video_touch_detected = 2\n");
                video_touch_detected = 2;
            }
            else if (input_x >= interval_width * 2 && input_x < interval_width * 3)
            {
                // 执行第三个操作
                printf("继续 video_touch_detected = 3\n");
                video_touch_detected = 3;
            }
            else if (input_x >= interval_width * 3 && input_x < interval_width * 4)
            {
                printf("暂停 video_touch_detected = 4\n");
                video_touch_detected = 4;
            }
            else if (input_x >= interval_width * 4 && input_x < interval_width * 5)
            {
                // 执行第四个操作
                // 下一个视频
                printf("下一个视频 video_touch_detected = 5\n");
                video_touch_detected = 5;
            }
            else if (input_x >= interval_width * 5 && input_x <= input_x_max)
            {
                // 执行第五个操作
                printf("结束播放 video_touch_detected = 6\n");
                video_touch_detected = 6;
            }
            else
            {
                // 点击了空白处，重新等待输入
                get_input("music");
            }
        }
        else if (input_y <= 120)
        {
            if (input_x >= interval_width * 5 && input_x <= input_x_max)
            {
                printf("结束播放 video_touch_detected = 7\n");
                video_touch_detected = 7;
                // 返回主页面
                if (video_is_playing = 1)
                {
                    system("killall -9 mplayer");
                    printf("杀视频\n");
                    usleep(100);
                    video_is_playing = 0;
                }
                printf("返回主界面");
                return NULL;
            }
        }
        else
        {
            get_input("music");
        }
    }
}

/*******************************************基础代码******************************************************/

/*
    功能: 显示800*480分辨率的bmp图片画面到lcd屏幕上
    参数: picname: 需要显示的图片名字
    返回值: -1: 打开屏幕失败,....
    调用示例: show_bmp("ggbond.bmp");
*/
int show_bmp(char *picname)
{
    // 1. 打开两个文件(bmp图片文件+lcd屏幕文件)
    int lcd_fd = open("/dev/fb0", O_RDWR);
    int bmp_fd = open(picname, O_RDWR);
    if (lcd_fd == -1)
    {
        printf("open lcd error\n");
        return -1;
    }

    if (bmp_fd == -1)
    {
        printf("open bmp error\n");
        return -2;
    }
    // if(bmp_fd == -1 || lcd_fd == -1)

    // 2. 读取bmp图片的颜色数据
    lseek(bmp_fd, 54, SEEK_SET); // 偏移文件的光标
    char buf[800 * 480 * 3];
    read(bmp_fd, buf, 800 * 480 * 3);

    // 3. 写如颜色数据到lcd中
    // 3.1 申请映射空间
    int *mmap_start = mmap(NULL, 800 * 480 * 4, PROT_READ | PROT_WRITE, MAP_SHARED, lcd_fd, 0);
    if (mmap_start == (void *)-1)
    {
        printf("mmap error\n");
        return -3;
    }

    // 3.2 使用映射空间写入颜色数据
    int n = 0;
    for (int y = 0; y < 480; y++)
    {
        for (int x = 0; x < 800; x++, n += 3)
        {
            *(mmap_start + 800 * (479 - y) + x) = buf[n + 0] << 0 |
                                                  buf[n + 1] << 8 |
                                                  buf[n + 2] << 16 |
                                                  0 << 24;
        }
    }

    // 3.3 撤销映射空间
    munmap(mmap_start, 800 * 480 * 4);

    // 4. 关闭两个文件
    close(lcd_fd);
    close(bmp_fd);
    return 0;
}

int new_show_bmp(char *picname, int show_x, int show_y)
{
    // 1. 打开两个文件(bmp图片文件+lcd屏幕文件)
    int lcd_fd = open("/dev/fb0", O_RDWR);
    int bmp_fd = open(picname, O_RDWR);
    if (lcd_fd == -1)
    {
        printf("open lcd error\n");
        return -1;
    }

    if (bmp_fd == -1)
    {
        printf("open bmp error\n");
        return -2;
    }
    // if(bmp_fd == -1 || lcd_fd == -1)

    // 2. 读取bmp图片的颜色数据
    int length, high;
    // 图片的长宽信息都在54字节中
    lseek(bmp_fd, 18, SEEK_SET);
    read(bmp_fd, &length, 4); // 读到了长
    read(bmp_fd, &high, 4);   // 读到了长
                              //  printf("长:%d, 宽:%d\n", length, high);

    lseek(bmp_fd, 54, SEEK_SET); // 偏移文件的光标
    char buf[length * high * 3];
    read(bmp_fd, buf, length * high * 3);

    // 3. 写如颜色数据到lcd中
    // 3.1 申请映射空间
    int *mmap_start = mmap(NULL, 800 * 480 * 4, PROT_READ | PROT_WRITE, MAP_SHARED, lcd_fd, 0);
    if (mmap_start == (void *)-1)
    {
        printf("mmap error\n");
        return -3;
    }

    // 3.2 使用映射空间写入颜色数据
    // 图片显示的起点
    int *new_p = mmap_start + 800 * show_y + show_x;

    int n = 0;
    for (int y = 0; y < high; y++)
    {
        for (int x = 0; x < length; x++, n += 3)
        {
            *(new_p + 800 * (high - 1 - y) + x) = buf[n + 0] << 0 |
                                                  buf[n + 1] << 8 |
                                                  buf[n + 2] << 16 |
                                                  0 << 24;
        }
    }

    // 3.3 撤销映射空间
    munmap(mmap_start, 800 * 480 * 4);

    // 4. 关闭两个文件
    close(lcd_fd);
    close(bmp_fd);
    return 0;
}
/*
    功能: // 当手指离开屏幕时,打印最后一次触摸的位置
    参数: 无
    返回值: ....
    调用示例: touch_fun();
*/
int touch_fun()
{
    // 1. 打开触摸屏文件
    int input_fd = open("/dev/input/event0", O_RDWR);
    if (input_fd == -1)
    {
        printf("open 触摸屏 error\n");
        return -1;
    }

    // 2. 读取数据
    struct input_event buf;
    int x, y;
    while (1)
    {
        // sizeof(int)	--> 4	求目标所占内存的字节数
        read(input_fd, &buf, sizeof(buf));
        // // 研究压力事件
        // if(buf.type==EV_KEY && buf.code==BTN_TOUCH && buf.value==0)
        // {
        // 	printf("未离开\n");
        // }
        // if(buf.type==EV_KEY && buf.code==BTN_TOUCH && buf.value==1)
        // {
        // 	printf("已离开\n");
        // }
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
            input_x = x;
            input_y = y;
            printf("已离开:%d, %d\n", input_x, input_y);
            break;
        }
    }

    // 3. 关闭文件
    close(input_fd);
    return 0;
}