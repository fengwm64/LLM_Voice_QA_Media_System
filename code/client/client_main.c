// #include "function.h"

// // 主函数
// int main()
// {
//     int photoNum = 4; 
//     char *photoName[] = {"1.bmp", "2.bmp", "3.bmp", "4.bmp"};
//     PhotoAlbum(photoNum, photoName);

//     return 0;
// }

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <linux/input.h>
#include <strings.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

void play_voice()
{
	//创建套接字
	int sock_fd = socket(PF_INET,SOCK_STREAM,0);
	printf("aaa\n");
	//绑定地址(IP+PORT)
	struct sockaddr_in srvaddr;
	srvaddr.sin_family = PF_INET;
	//端口
	srvaddr.sin_port = htons(7777);		
	//IP地址
	srvaddr.sin_addr.s_addr = inet_addr("192.168.100.250");	
	printf("002\n");
	//发起连接请求
	connect(sock_fd,(struct sockaddr *)&srvaddr,sizeof(srvaddr));
	printf("003\n");
	int fd = open("1.wav",O_RDWR);
	if(fd < 0)
	{
		perror("open() failed");
		return;
	}
	//获取文件总字节数
	int file_size = lseek(fd,0,SEEK_END);
	//将文件光标偏移到文件开头
	lseek(fd,0,SEEK_SET);
	send(sock_fd,&file_size,4,0);
	printf("file_size = %d\n",file_size);
	
	char buf[1024] = {0};
	int ret_rd;
	while(1)
	{
		bzero(buf,1024);
		ret_rd = read(fd,buf,1024);
		printf("ret_rd = %d\n",ret_rd);
		if(ret_rd == 0)	//发送完毕
		{
			printf("发送完毕\n");
			break;
		}
		send(sock_fd,buf,ret_rd,0);
		if(ret_rd < 1024)
		{
			printf("send over\n");
			break;
		}
	}
	bzero(buf,100);
	printf("等待接收数据\n");
	recv(sock_fd,buf,100,0);
	if(strstr(buf,"相册") != NULL)  //匹配相册成功
	{
		//play_photo();
		printf("play photo\n");
	}
	if(strstr(buf,"音乐") != NULL)  //匹配音乐成功
	{
		//play_music();
		printf("play music\n");
	}
	if(strstr(buf,"视频") != NULL)  //匹配视频成功
	{
		//play_video();
		printf("play video\n");
	}
	if(strstr(buf,"没有识别结果") != NULL)
	{
		printf("没有识别结果");
	}
	//关闭套接字
	close(sock_fd);
	close(fd);
}

int main()
{
	printf("开始录音\n");
	system("arecord -d3 -c1 -r16000 -twav -fS16_LE 1.wav");
	play_voice();	
	return 0;
}