#include "agent.h"

#define BUFFER_SIZE 8192
#define LLM_AUDIO_RES "res.wav"
#define LLM_WAKE "科学科学"

// TOOLS
// ============================================================= //
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

// 随机等待函数
void wait_ran_time(int min, int max)
{
    if (min > max)
    {
        fprintf(stderr, "错误：min 应该小于或等于 max\n");
        return;
    }

    // 初始化随机数生成器
    srand(time(NULL));

    // 生成 [min, max] 范围内的随机数
    int wait_time = min + rand() % (max - min + 1);

    // 调用 usleep 函数
    usleep(wait_time);
}

// 清空画板并返回新的画板对象
bitmap *clear_bitmap(bitmap *bm)
{
    if (bm != NULL)
    {
        destroyBitmap(bm);
    }
    return createBitmapWithInit(800, 80, 4, getColor(0, 255, 255, 255));
}

// 初始化客户端
int socket_Init()
{
    // 创建套接字
    int sock_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0)
    {
        perror("socket() failed");
        return -1;
    }
    printf("Socket created\n");

    // 定义服务器地址结构
    struct sockaddr_in srvaddr;
    srvaddr.sin_family = PF_INET;                           // 地址族
    srvaddr.sin_port = htons(7777);                         // 端口号
    srvaddr.sin_addr.s_addr = inet_addr("192.168.100.250"); // IP地址
    printf("Address set\n");

    // 发起连接请求
    if (connect(sock_fd, (struct sockaddr *)&srvaddr, sizeof(srvaddr)) < 0)
    {
        perror("connect() failed");
        close(sock_fd);
        return -1;
    }
    printf("Connected to server\n");

    return sock_fd;
}

// 接收文件
int receive_File(int sock_fd, const char *filename)
{
    int fd = open(filename, O_RDWR | O_CREAT, 0777);
    if (fd < 0)
    {
        perror("open() failed");
        return -1;
    }

    // 接收文件总字节数
    int file_size;
    if (recv(sock_fd, &file_size, sizeof(file_size), 0) < 0)
    {
        perror("recv() failed");
        close(fd);
        return -1;
    }
    printf("File size = %d bytes\n", file_size);

    char buffer[BUFFER_SIZE];
    int recv_size, total_size = 0;

    // 接收文件内容
    while (total_size < file_size)
    {
        recv_size = recv(sock_fd, buffer, sizeof(buffer), 0);
        if (recv_size < 0)
        {
            perror("recv() failed");
            close(fd);
            return -1;
        }
        else if (recv_size == 0)
        {
            printf("Client disconnected\n");
            close(fd);
            return -1;
        }

        // 写入接收到的数据到文件
        if (write(fd, buffer, recv_size) < 0)
        {
            perror("write() failed");
            close(fd);
            return -1;
        }

        total_size += recv_size;
    }

    printf("File received successfully\n");

    close(fd);
    return 0;
}

// 发送文件
int send_File(int sock_fd, const char *filename)
{
    // 打开要发送的文件
    int send_fd = open(filename, O_RDONLY);
    if (send_fd < 0)
    {
        perror("open() failed");
        return -1;
    }

    // 获取文件大小
    int file_size = lseek(send_fd, 0, SEEK_END);
    lseek(send_fd, 0, SEEK_SET);

    // 发送文件大小
    if (send(sock_fd, &file_size, sizeof(file_size), 0) < 0)
    {
        perror("send() failed");
        close(send_fd);
        return -1;
    }
    printf("Sending file size: %d bytes\n", file_size);

    // 发送文件内容
    char send_buffer[BUFFER_SIZE];
    int send_size, total_sent = 0;
    while ((send_size = read(send_fd, send_buffer, sizeof(send_buffer))) > 0)
    {
        int sent = send(sock_fd, send_buffer, send_size, 0);
        if (sent < 0)
        {
            perror("send() failed");
            close(send_fd);
            return -1;
        }
        total_sent += sent;
    }

    // 关闭文件
    close(send_fd);
    printf("File sent successfully\n");

    return 0;
}

// 接收消息
char *receive_Msg(int sock_fd)
{
    char buf[BUFFER_SIZE];

    memset(buf, 0, sizeof(buf)); // 清空接收缓冲区

    if (recv(sock_fd, buf, sizeof(buf) - 1, 0) < 0)
    {
        perror("recv() failed");
        close(sock_fd);
        return NULL;
    }

    printf("Response received: %s\n", buf);

    return strdup(buf);
}

// 接收文字

// Agent
// ============================================================= //
// 初始化Agent
void agent_Init(struct Agent *agent, const char **bmp_files, int bmp_num, const char *message, int line)
{
    agent->bmp_files = bmp_files;
    agent->idx = 0;
    agent->MAX_IDX = bmp_num - 1;
    agent->MAX_LINE = line;

    // 显示agent形象
    agent_Update(agent);

    // 显示欢迎信息
    agent_Speak(agent, message);
}

// 切换Agent显示图片
void agent_Update(struct Agent *agent)
{
    // 确保索引在有效范围内
    agent->idx = (agent->idx + 1) % (agent->MAX_IDX + 1);

    // 显示下一张BMP
    displayBMP(agent->bmp_files[agent->idx], 0, 0);
}

// 显示信息
void agent_Speak(struct Agent *agent, const char *message)
{
    if (!message)
    {
        printf("Agent speak message is NULL!\n");
        return;
    }

    printf("Agent Speak: %s\n", message);

    // 初始化LCD设备
    struct LcdDevice *lcd = lcd_init();

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

    for (int i = 0, px = 0; i < len; i += utf8_char_length(message[i]), px += 65)
    {
        agent_Update(agent);

        if (n <= agent->MAX_LINE)
        {
            // 复制当前字符到displayText中
            int char_len = utf8_char_length(message[i]);
            memcpy(displayText, &message[i], char_len);
            displayText[char_len] = '\0';

            // 将字体打印到点阵图上
            fontPrint(f, bm, px, 0, displayText, getColor(0, 100, 100, 100), 0);

            // 将点阵图显示到LCD屏幕上
            show_font_to_lcd(lcd->mp, 0, 400, bm);

            // 等待，模拟逐字输出效果
            printf("wait...\n");
            wait_ran_time(100000, 150000);
        }
        else
        {
            printf("Clearing bitmap...\n");

            // 重新初始化画板为背景色
            bm = clear_bitmap(bm);
            n = 0; // 重置计数器
            px = 0;

            // 复制当前字符到displayText中
            int char_len = utf8_char_length(message[i]);
            memcpy(displayText, &message[i], char_len);
            displayText[char_len] = '\0';

            // 将字体打印到点阵图上
            fontPrint(f, bm, px, 0, displayText, getColor(0, 100, 100, 100), 0);

            // 将点阵图显示到LCD屏幕上
            show_font_to_lcd(lcd->mp, 0, 400, bm);

            // 等待，模拟逐字输出效果
            printf("wait...\n");
            wait_ran_time(90000, 500000);
            continue;
        }

        n++;
    }

    // 卸载字体库
    fontUnload(f);

    // 销毁画板
    destroyBitmap(bm);

    // 关闭 LCD 设备
    lcd_close(lcd);
}

// 语音控制
char *speak_to_Agent(const char *record_name)
{
    // 创建套接字
    int sock_fd = socket_Init();

    // 发送文件
    send_File(sock_fd, record_name);

    // 准备接收服务器响应
    printf("Waiting for response\n");

    // ================= 接收文字 ======================
    char * buf = receive_Msg(sock_fd);

    // ================= 先接收音频 ======================
    receive_File(sock_fd, LLM_AUDIO_RES);

    // 处理响应内容
    if (strstr(buf, "打开相册") != NULL)
    {
        // play_photo();
        printf("Playing photo\n");
        return strdup("photo");
    }
    else if (strstr(buf, "打开音乐") != NULL)
    {
        // play_music();
        printf("Playing music\n");
        return strdup("music");
    }
    else if (strstr(buf, "打开视频") != NULL)
    {
        // play_video();
        printf("Playing video\n");
        return strdup("video");
    }
    else
    {
        printf("播放语音\n");
        system("nohup aplay res.wav &");
    }

    // 关闭套接字和文件描述符
    close(sock_fd);
    printf("Resources released\n");

    return buf;
}