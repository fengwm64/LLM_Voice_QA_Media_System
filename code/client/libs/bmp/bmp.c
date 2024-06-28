#include "bmp.h"

// 显示 BMP 图像到 LCD 屏幕上
int displayBMP(const char *bmp_path, int offset_x, int offset_y)
{
	struct LcdDevice *lcd = lcd_init();
	if (lcd == NULL)
	{
		fprintf(stderr, "初始化LCD设备失败\n");
		return -1;
	}

	// 打开 BMP 图像文件
	int fd_bmp = open(bmp_path, O_RDWR);
	if (fd_bmp < 0)
	{
		perror("打开BMP图像失败");
		lcd_close(lcd);
		return -1;
	}

	// 读取 BMP 图像的宽度和高度
	int width, height;
	lseek(fd_bmp, 18, SEEK_SET);
	read(fd_bmp, &width, 4);
	read(fd_bmp, &height, 4);

	// 跳过 BMP 文件头
	lseek(fd_bmp, 54, SEEK_SET);

	// 读取 BMP 数据
	char *buf = malloc(width * height * 3);
	if (buf == NULL)
	{
		fprintf(stderr, "分配内存失败: %s\n", strerror(errno));
		close(fd_bmp);
		lcd_close(lcd);
		return -1;
	}
	read(fd_bmp, buf, width * height * 3);

	// 将 BMP 数据写入 LCD
	int *fb = (int *)lcd->mp;
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			fb[(offset_y + height - 1 - y) * 800 + offset_x + x] =
				buf[(y * width + x) * 3] |
				buf[(y * width + x) * 3 + 1] << 8 |
				buf[(y * width + x) * 3 + 2] << 16;
		}
	}

	// 清理资源
	free(buf);
	close(fd_bmp);
	lcd_close(lcd);

	return 0;
}