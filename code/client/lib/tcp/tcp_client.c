#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "font.h" // 自定义的字体库头文件

#define BUFFER_SIZE 1024
#define LINE 10
#define MAX_IMAGES 3

const char *bmp_files[MAX_IMAGES] = {
    "1.bmp",
    "2.bmp",
    "3.bmp"};

// 当前显示的图像索引
int currentImageIndex = 0;

// 初始化LCD设备
struct LcdDevice *init_lcd(const char *device)
{
    struct LcdDevice *lcd = malloc(sizeof(struct LcdDevice));
    if (lcd == NULL)
    {
        return NULL;
    }

    lcd->fd = open(device, O_RDWR);
    if (lcd->fd < 0)
    {
        perror("打开LCD设备失败");
        free(lcd);
        return NULL;
    }

    lcd->mp = mmap(NULL, 800 * 480 * 4, PROT_READ | PROT_WRITE, MAP_SHARED, lcd->fd, 0);
    return lcd;
}

// 计算UTF-8编码字符的长度（字节数）
int utf8_char_length(unsigned char c)
{
    if ((c & 0x80) == 0x00) // 0xxxxxxx，单字节字符
        return 1;
    else if ((c & 0xE0) == 0xC0) // 110xxxxx，双字节字符
        return 2;
    else if ((c & 0xF0) == 0xE0) // 1110xxxx，三字节字符
        return 3;
    else if ((c & 0xF8) == 0xF0) // 11110xxx，四字节字符
        return 4;
    else
        return 1; // 非法字符，按单字节处理
}

// 显示BMP图像到LCD屏幕上
int displayBMP(const char *bmp_path, int offset_x, int offset_y)
{
    // 打开LCD设备
    int fd_lcd = open("/dev/fb0", O_RDWR);
    if (fd_lcd < 0)
    {
        perror("打开LCD设备失败");
        return -1;
    }

    // 内存映射LCD设备
    int *addr = mmap(NULL, 800 * 480 * 4, PROT_READ | PROT_WRITE, MAP_SHARED, fd_lcd, 0);
    if (addr == MAP_FAILED)
    {
        perror("内存映射失败");
        close(fd_lcd);
        return -1;
    }

    // 打开BMP图像文件
    int fd_bmp = open(bmp_path, O_RDWR);
    if (fd_bmp < 0)
    {
        perror("打开BMP图像失败");
        munmap(addr, 800 * 480 * 4);
        close(fd_lcd);
        return -1;
    }

    // 读取BMP图像的宽度和高度
    int width, height;
    lseek(fd_bmp, 18, SEEK_SET);
    read(fd_bmp, &width, 4);
    read(fd_bmp, &height, 4);
    printf("BMP宽度：%d，高度：%d\n", width, height);

    // 跳过BMP文件头
    lseek(fd_bmp, 54, SEEK_SET);

    // 读取BMP数据
    char buf[width * height * 3];
    read(fd_bmp, buf, sizeof(buf));

    // 将BMP数据写入LCD
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            *(addr + (offset_y + height - 1 - y) * 800 + offset_x + x) =
                buf[(y * width + x) * 3] |
                buf[(y * width + x) * 3 + 1] << 8 |
                buf[(y * width + x) * 3 + 2] << 16;
        }
    }

    // 清理资源
    munmap(addr, 800 * 480 * 4);
    close(fd_bmp);
    close(fd_lcd);

    return 0;
}

// 切换显示下一张BMP图像
void switchBMP(void)
{
    // 确保索引在有效范围内
    currentImageIndex = (currentImageIndex + 1) % MAX_IMAGES;

    // 显示下一张BMP
    displayBMP(bmp_files[currentImageIndex], 0, 0);
}

// 在LCD屏幕上显示消息
void showMessage(char *message)
{
    // 初始化LCD设备
    struct LcdDevice *lcd = init_lcd("/dev/fb0");

    // 加载字体库
    font *f = fontLoad("/usr/share/fonts/simkai.ttf");

    // 设置字体大小
    fontSetSize(f, 72);

    // 创建画板（点阵图）
    bitmap *bm = createBitmapWithInit(800, 80, 4, getColor(0, 255, 255, 255));

    int len = strlen(message);
    int n = 0;

    // 准备显示的文本
    char displayText[4] = {'\0'}; // 最多支持显示两个中文字符

    // 随机数生成器初始化
    static bool first_time = true;
    if (first_time)
    {
        srand(time(NULL));
        first_time = false;
    }

    for (int i = 0, px = 0; i < len; i += utf8_char_length(message[i]), px += 65)
    {
        switchBMP(); // 切换显示下一张BMP图像

        if (n <= LINE)
        {
            // 复制当前字符到displayText中
            int char_len = utf8_char_length(message[i]);
            memcpy(displayText, &message[i], char_len);
            displayText[char_len] = '\0';

            // 将字体打印到点阵图上
            fontPrint(f, bm, px, 0, displayText, getColor(0, 100, 100, 100), 0);

            // 将点阵图显示到LCD屏幕上
            show_font_to_lcd(lcd->mp, 0, 400, bm);

            // 生成随机等待时间
            int random_usleep_duration = 100000 + rand() % (300000 - 100000 + 1);
            usleep(random_usleep_duration);
        }
        else
        {
            printf("清空画板\n");

            // 重新初始化画板为背景色
            destroyBitmap(bm);                                                 // 销毁原有的位图
            bm = createBitmapWithInit(800, 80, 4, getColor(0, 255, 255, 255)); // 重新创建带背景色的位图
            n = 0;                                                             // 重置计数器
            px = 0;

            // 复制当前字符到displayText中
            int char_len = utf8_char_length(message[i]);
            memcpy(displayText, &message[i], char_len);
            displayText[char_len] = '\0';

            // 将字体打印到点阵图上
            fontPrint(f, bm, px, 0, displayText, getColor(0, 100, 100, 100), 0);

            // 将点阵图显示到LCD屏幕上
            show_font_to_lcd(lcd->mp, 0, 400, bm);

            // 固定等待时间
            usleep(300000);
            continue;
        }

        n++;
    }

    // 卸载字体库
    fontUnload(f);

    // 销毁画板
    destroyBitmap(bm);

    // 释放LCD资源
    if (lcd != NULL)
    {
        munmap(lcd->mp, 800 * 480 * 4);
        close(lcd->fd);
        free(lcd);
    }
}

// 主函数
int main()
{
    // 创建客户端通信对象
    int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_socket < 0)
    {
        perror("创建客户端对象失败");
        return -1;
    }
    else
    {
        printf("创建客户端对象成功\n");
    }

    // 连接服务器
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(7777);                       // 设置端口为7777
    server_addr.sin_addr.s_addr = inet_addr("192.168.5.250"); // 设置服务器IP地址

    // 连接服务器
    int ret = connect(tcp_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret < 0)
    {
        perror("连接服务器失败");
        close(tcp_socket);
        return -1;
    }
    else
    {
        printf("连接服务器成功\n");
    }

    // 打开待发送的音频文件
    FILE *file = fopen("./file/cmd.wav", "rb");
    if (file == NULL)
    {
        perror("打开文件失败");
        close(tcp_socket);
        return -1;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;

    // 循环读取并发送文件内容
    while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, file)) > 0)
    {
        write(tcp_socket, buffer, bytesRead);
    }

    fclose(file);
    printf("文件发送完成\n");

    // 关闭客户端的写端，以通知服务器文件发送完毕
    shutdown(tcp_socket, SHUT_WR);

    // 接收服务器的确认消息
    char confirmation_message[BUFFER_SIZE];
    ssize_t msg_len = read(tcp_socket, confirmation_message, BUFFER_SIZE);
    if (msg_len > 0)
    {
        confirmation_message[msg_len] = '\0';
        printf("收到服务器消息: %s\n", confirmation_message);

        // 在LCD屏幕上显示消息
        showMessage(confirmation_message);
    }
    else
    {
        perror("接收服务器消息失败");
    }

    // 关闭客户端通信对象
    close(tcp_socket);

    return 0;
}
