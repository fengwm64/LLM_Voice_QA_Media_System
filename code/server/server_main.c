/*
 * 语音听写(iFly Auto Transform)技术能够实时地将语音转换成对应的文字。
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "qisr.h"
#include "msp_cmn.h"
#include "msp_errors.h"

#define BUFFER_SIZE 8192
#define FRAME_LEN 640
#define HINTS_SIZE 100
#define RECORD_NAME "wav/cmd.wav"
#define AUDIO_CMD "ekho"
#define LLM_CMD "python ../LLM/chat.py"
#define LLM_WAKE "科学科学"
#define LLM_TEXT_RES "res.txt"
#define LLM_AUDIO_RES "res.wav"

char *run_iat(const char *audio_file, const char *session_begin_params)
{
	const char *session_id = NULL;
	char rec_result[BUFFER_SIZE] = {NULL};
	char hints[HINTS_SIZE] = {NULL}; // hints为结束本次会话的原因描述，由用户自定义
	unsigned int total_len = 0;
	int aud_stat = MSP_AUDIO_SAMPLE_CONTINUE; // 音频状态
	int ep_stat = MSP_EP_LOOKING_FOR_SPEECH;  // 端点检测
	int rec_stat = MSP_REC_STATUS_SUCCESS;	  // 识别状态
	int errcode = MSP_SUCCESS;

	FILE *f_pcm = NULL;
	char *p_pcm = NULL;
	long pcm_count = 0;
	long pcm_size = 0;
	long read_size = 0;

	if (NULL == audio_file)
		goto iat_exit;

	f_pcm = fopen(audio_file, "rb");
	if (NULL == f_pcm)
	{
		printf("\nopen [%s] failed! \n", audio_file);
		goto iat_exit;
	}

	fseek(f_pcm, 0, SEEK_END);
	pcm_size = ftell(f_pcm); // 获取音频文件大小
	fseek(f_pcm, 0, SEEK_SET);

	p_pcm = (char *)malloc(pcm_size);
	if (NULL == p_pcm)
	{
		printf("\nout of memory! \n");
		goto iat_exit;
	}

	read_size = fread((void *)p_pcm, 1, pcm_size, f_pcm); // 读取音频文件内容
	if (read_size != pcm_size)
	{
		printf("\nread [%s] error!\n", audio_file);
		goto iat_exit;
	}

	printf("\n开始语音听写 ...\n");
	session_id = QISRSessionBegin(NULL, session_begin_params, &errcode); // 听写不需要语法，第一个参数为NULL
	if (MSP_SUCCESS != errcode)
	{
		printf("\nQISRSessionBegin failed! error code:%d\n", errcode);
		goto iat_exit;
	}

	while (1)
	{
		unsigned int len = 10 * FRAME_LEN; // 每次写入200ms音频(16k，16bit)：1帧音频20ms，10帧=200ms。16k采样率的16位音频，一帧的大小为640Byte
		int ret = 0;

		if (pcm_size < 2 * len)
			len = pcm_size;
		if (len <= 0)
			break;

		aud_stat = MSP_AUDIO_SAMPLE_CONTINUE;
		if (0 == pcm_count)
			aud_stat = MSP_AUDIO_SAMPLE_FIRST;

		printf(">");
		ret = QISRAudioWrite(session_id, (const void *)&p_pcm[pcm_count], len, aud_stat, &ep_stat, &rec_stat);
		if (MSP_SUCCESS != ret)
		{
			printf("\nQISRAudioWrite failed! error code:%d\n", ret);
			goto iat_exit;
		}

		pcm_count += (long)len;
		pcm_size -= (long)len;

		if (MSP_REC_STATUS_SUCCESS == rec_stat) // 已经有部分听写结果
		{
			const char *rslt = QISRGetResult(session_id, &rec_stat, 0, &errcode);
			if (MSP_SUCCESS != errcode)
			{
				printf("\nQISRGetResult failed! error code: %d\n", errcode);
				goto iat_exit;
			}
			if (NULL != rslt)
			{
				unsigned int rslt_len = strlen(rslt);
				total_len += rslt_len;
				if (total_len >= BUFFER_SIZE)
				{
					printf("\nno enough buffer for rec_result !\n");
					goto iat_exit;
				}
				strncat(rec_result, rslt, rslt_len);
			}
		}

		if (MSP_EP_AFTER_SPEECH == ep_stat)
			break;
		usleep(200 * 1000); // 模拟人说话时间间隙。200ms对应10帧的音频
	}
	errcode = QISRAudioWrite(session_id, NULL, 0, MSP_AUDIO_SAMPLE_LAST, &ep_stat, &rec_stat);
	if (MSP_SUCCESS != errcode)
	{
		printf("\nQISRAudioWrite failed! error code:%d \n", errcode);
		goto iat_exit;
	}

	while (MSP_REC_STATUS_COMPLETE != rec_stat)
	{
		const char *rslt = QISRGetResult(session_id, &rec_stat, 0, &errcode);
		if (MSP_SUCCESS != errcode)
		{
			printf("\nQISRGetResult failed, error code: %d\n", errcode);
			goto iat_exit;
		}
		if (NULL != rslt)
		{
			unsigned int rslt_len = strlen(rslt);
			total_len += rslt_len;
			if (total_len >= BUFFER_SIZE)
			{
				printf("\nno enough buffer for rec_result !\n");
				goto iat_exit;
			}
			strncat(rec_result, rslt, rslt_len);
		}
		usleep(150 * 1000); // 防止频繁占用CPU
	}
	printf("\n语音听写结束\n");
	printf("=============================================================\n");
	printf("%s\n", rec_result);
	printf("=============================================================\n");

	char *result = strdup(rec_result); // 动态分配内存并复制结果字符串

iat_exit:
	if (NULL != f_pcm)
	{
		fclose(f_pcm);
		f_pcm = NULL;
	}
	if (NULL != p_pcm)
	{
		free(p_pcm);
		p_pcm = NULL;
	}

	QISRSessionEnd(session_id, hints);

	return result; // 返回结果字符串
}

int socket_Init()
{
	int sock_fd;

	// 创建套接字
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_fd < 0)
	{
		perror("socket() failed");
		return -1;
	}

	// 设置端口重用
	int on = 1;
	if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
	{
		perror("setsockopt() failed");
		close(sock_fd);
		return -1;
	}

	// 绑定地址(IP+PORT)
	struct sockaddr_in srvaddr;
	srvaddr.sin_family = AF_INET;
	srvaddr.sin_port = htons(7777);
	srvaddr.sin_addr.s_addr = inet_addr("192.168.100.250");

	if (bind(sock_fd, (struct sockaddr *)&srvaddr, sizeof(srvaddr)) < 0)
	{
		perror("bind() failed");
		close(sock_fd);
		return -1;
	}

	// 设置监听套接字
	if (listen(sock_fd, 4) < 0)
	{
		perror("listen() failed");
		close(sock_fd);
		return -1;
	}

	return sock_fd;
}

int receive_File(int conn_fd, const char *filename)
{
	int fd = open(filename, O_RDWR | O_CREAT, 0777);
	if (fd < 0)
	{
		perror("open() failed");
		return -1;
	}

	// 接收文件总字节数
	int file_size;
	if (recv(conn_fd, &file_size, sizeof(file_size), 0) < 0)
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
		recv_size = recv(conn_fd, buffer, sizeof(buffer), 0);
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

// 发送文件函数定义
int send_File(int conn_fd, const char *filename)
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
	if (send(conn_fd, &file_size, sizeof(file_size), 0) < 0)
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
		int sent = send(conn_fd, send_buffer, send_size, 0);
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

int send_Msg(int conn_fd, const char *msg)
{
	if (send(conn_fd, msg, strlen(msg), 0) == -1)
	{
		perror("send() failed");
	}
	printf("\n");
	printf("文字已经发送到开发板\n");
}

char *extract_Question(char *rec_result)
{
	// 寻找第一次出现唤醒词的位置
	char *ptr = strstr(rec_result, LLM_WAKE);

	// 获取第一次出现唤醒词后的所有字符
	ptr += strlen(LLM_WAKE);

	// 如果唤醒词后面有逗号，也将其跳过
	if (*ptr == ',')
		ptr++;

	// 跳过可能存在的空格
	while (*ptr == ' ')
		ptr++;

	// 复制剩余的字符串
	return strdup(ptr);
}

char *extract_Answer(char *buffer)
{
	char res[BUFFER_SIZE];
	char *start, *end;
	// 找到content=的起始位置
	start = strstr(buffer, "content='");
	if (start)
	{
		start += 9; // 跳过"content='"这个字符串
		// 找到结束的单引号
		end = strstr(start, "'");
		if (end)
		{
			// 计算内容的长度
			size_t len = end - start;
			// 复制内容到res
			strncpy(res, start, len);
			res[len] = '\0'; // 添加字符串结束符
		}
	}

	return strdup(res);
}

void text_To_File(char *res)
{
	FILE *file = fopen(LLM_TEXT_RES, "w");
	if (file == NULL)
	{
		fprintf(stderr, "Error opening file for writing.\n");
		return 1;
	}
	fprintf(file, "%s\n", res);
	fclose(file);
}

void text_To_Audio()
{
	char command[256];
	sprintf(command, "%s %s %s %s %s %s", AUDIO_CMD, "-f", LLM_TEXT_RES, "-o", LLM_AUDIO_RES, "-s -10");
	printf("Command: %s\n", command);
	system(command);
	sleep(1);
	printf("done command\n");
}

FILE *load_LLM(char *question)
{
	// 拼接指令
	char cmd[4096];
	snprintf(cmd, sizeof(cmd), "%s %s", LLM_CMD, question);

	// 加载模型
	FILE *fp = popen(cmd, "r");
	if (fp == NULL)
	{
		perror("无法加载模型\n");
		return NULL;
	}

	return fp;
}

char *get_LLM_Output(FILE *fp)
{
	char buffer[BUFFER_SIZE];
	while (1)
	{
		if (fgets(buffer, sizeof(buffer), fp) == NULL)
		{
			printf("加载完毕\n");
			break;
		}
		printf("%s", buffer);
	}

	return strdup(buffer);
}

int main(int argc, char *argv[])
{
	int ret = MSP_SUCCESS;
	const char *login_params = "appid = e2ba452f, work_dir = ."; // 登录参数，appid与msc库绑定,请勿随意改动

	/*
	 * sub:				请求业务类型
	 * domain:			领域
	 * language:			语言
	 * accent:			方言
	 * sample_rate:		音频采样率
	 * result_type:		识别结果格式
	 * result_encoding:	结果编码格式
	 *
	 */
	const char *session_begin_params = "sub = iat, domain = iat, language = zh_cn, accent = mandarin, sample_rate = 16000, result_type = plain, result_encoding = utf8";

	/* 用户登录 */
	ret = MSPLogin(NULL, NULL, login_params); // 第一个参数是用户名，第二个参数是密码，均传NULL即可，第三个参数是登录参数
	if (MSP_SUCCESS != ret)
	{
		printf("MSPLogin failed , Error code %d.\n", ret);
		goto exit; // 登录失败，退出登录
	}

	int sock_fd = socket_Init();
	int conn_fd = -1;

	while (1)
	{
		// ============== 等待客户端连接 ==============
		printf("等待连接\n");
		conn_fd = -1;
		conn_fd = accept(sock_fd, NULL, NULL);
		printf("连接成功\n");

		receive_File(conn_fd, RECORD_NAME);

		// ============== 进行语音转文字 ==============
		char *rec_result = run_iat(RECORD_NAME, session_begin_params);

		// ============== 分析语音内容 ==============
		if (rec_result != NULL)
		{
			// --------- 调用大模型 ---------
			if (strstr(rec_result, LLM_WAKE) != NULL)
			{
				printf("调用智谱大模型\n");
				// ----- 提取问题 -----
				char *question = extract_Question(rec_result);

				if (question == NULL)
					goto exit;

				printf("问题是：%s\n", question);

				// --------- 加载chatglm模型 ---------
				FILE *fp = load_LLM(question);

				// --------- 读取模型的输出内容 ---------
				char *buffer = get_LLM_Output(fp);

				// --------- 解析内容 ---------
				char *res = extract_Answer(buffer);
				printf("LLM回复解析结果为：%s\n", res);

				// --------- 将res写入res.txt文件 ---------
				text_To_File(res);

				// --------- 回答文字转语音 ---------
				text_To_Audio();

				// --------- 发送文字到开发板 ---------
				send_Msg(conn_fd, res);

				// ---------发送语音到开发板 ---------
				send_File(conn_fd, LLM_AUDIO_RES);

				free(question);
				free(res);
				free(buffer);
				pclose(fp);
			}

			// 指令，发送识别结果回客户端
			else
			{
				send_Msg(conn_fd, rec_result);
			}
		}
		free(rec_result);
		// 关闭套接字
		close(conn_fd);
		sleep(2);
	}

exit:
	printf("按任意键退出 ...\n");
	getchar();
	MSPLogout(); // 退出登录

	// 关闭套接字
	close(conn_fd);
	close(sock_fd);

	return 0;
}
