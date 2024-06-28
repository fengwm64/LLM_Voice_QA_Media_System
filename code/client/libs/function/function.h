#include "../../includes.h"


void ai_Agent();

int show_bmp(char *picname);
int touch_fun();
void get_input(char *name);
void *touch_detection(void *arg);

// 相册
void photo_play();
void manual();
int hand();
void *autom(void *arg); 

// 音乐
void music_number();
void switch_music(int flag);
void music_play();

// 视频
int video_play();
void* video_number();
void* touch_detection(void* arg);
void* video_touch_detection(void* arg);
void video_number_play();
