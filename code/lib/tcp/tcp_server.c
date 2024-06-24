#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define BUFFER_SIZE 1024 // 定义缓冲区大小

int main()
{
    // 1. 创建服务器对象
    int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_socket < 0)
    {
        perror("创建服务器对象失败");
        return -1;
    }
    else
    {
        printf("创建服务器对象成功\n");
    }

    // 2. 绑定服务器的 IP 地址和端口
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;                         // 设置地址族为 IPv4
    server_addr.sin_port = htons(7777);                       // 设置绑定的端口号为 7777
    server_addr.sin_addr.s_addr = inet_addr("192.168.5.250"); // 设置绑定的 IP 地址

    int ret = bind(tcp_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret < 0)
    {
        perror("绑定服务器失败");
        close(tcp_socket);
        return -1;
    }
    else
    {
        printf("绑定服务器成功\n");
    }

    // 3. 设置服务器为监听模式
    ret = listen(tcp_socket, 5); // 设置最大监听队列长度为 5
    if (ret < 0)
    {
        perror("监听失败");
        close(tcp_socket);
        return -1;
    }
    else
    {
        printf("监听成功\n");
    }

    // 4. 接收客户端的连接请求
    printf("等待客户端连接\n");
    int new_socket = accept(tcp_socket, NULL, NULL); // 接收客户端连接
    if (new_socket < 0)
    {
        perror("接收客户端连接失败");
        close(tcp_socket);
        return -1;
    }
    else
    {
        printf("新的客户端连接=%d\n", new_socket);
    }

    // 5. 接收客户端发送的文件数据
    FILE *file = fopen("received_cmd.wav", "wb"); // 打开或创建文件，用于保存接收的数据
    if (file == NULL)
    {
        perror("文件创建失败");
        close(new_socket);
        close(tcp_socket);
        return -1;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;
    // 循环读取客户端发送的数据，并写入文件
    while ((bytesRead = read(new_socket, buffer, BUFFER_SIZE)) > 0)
    {
        fwrite(buffer, 1, bytesRead, file);
    }

    fclose(file); // 关闭文件
    if (bytesRead < 0)
    {
        perror("接收文件数据失败");
    }
    else
    {
        printf("文件接收成功\n");

        // 6. 发送确认消息给客户端
        const char *confirmation_message = "我是你的智能语音助手，有什么问题请随时找我吧！";
        write(new_socket, confirmation_message, strlen(confirmation_message)); // 发送确认消息
    }

    // 关闭所有通信对象
    close(new_socket); // 关闭与客户端的连接
    close(tcp_socket); // 关闭服务器的监听套接字

    return 0;
}
