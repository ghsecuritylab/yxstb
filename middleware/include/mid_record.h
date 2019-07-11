
#ifndef __MID_RECORD_H__
#define __MID_RECORD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "mid_stream.h"
#include "ind_pvr.h"

void mid_record_init(void);

void mid_record_mount(void);
void mid_record_unmount(void);

/*
	录制复用当前播放流
	默认是开启的，国外频道均为单播，可以关闭该功能，间或处理流程
 */
void mid_record_mix(int mix);

/*
	设置预留空间
	最大码率 bitrate
	本地时移时长 second
 */
void mid_record_set_reserve(unsigned int bitrate, unsigned int second);
/*
	获取预留空间，以M为单位
 */
unsigned int mid_record_get_reserve(unsigned int total);


/*
	调用 mid_record_suffix(".in", ".ts"); 之后
	录制目录中的 info media0 文件就会变成 info.in media0.ts
 */
void mid_record_suffix(char* sinfo, char* smedia);

/*
录制过程中出现异常情况，会调用record_port_message来通知上层，其中有一个参数就为index。
int record_port_message(int pIndex, unsigned int id, RECORD_MSG msg, int arg);
 */

/*
	后台录制限定为两路，而每一路分别可支持RTSP,HTTP
	后台录制和播放是分离的，按照目前架构设计，STB同时两路RTSP播放和两路RTSP录制的并行（4路RTSP可以并行）
 */
/*
	后台开始录制
	index：
		录制路索引 0 或 1，同当两路播放的index意义相近
	url：录制路径
		合法的格式有rtsp://xxx,igmp://xxx,http://xxx
	info_buf：
		录制节目信息，或继续录制时的本地URL。
	info_len：
		录制节目信息长度，长度为8表明为继续录制。
	id:
		续传的录制ID

		begin, end 录制起始时间和录制结束时间，对于没有明确范围的及时录制，应该传0
	返回值：
		录制ID
 */
unsigned int mid_record_open(int index, char* url, APP_TYPE apptype, char *info_buf, int info_len, unsigned int id, unsigned int begin, unsigned int end);

/*
	先由mid_record_open0生成录制ID，然后再由mid_record_open1启动录制任务，这样避免mid_record_open调用过程中产生录制消息而无录制ID对应。
 */
unsigned int mid_record_open0(int index, char* url, APP_TYPE apptype, char* info_buf, int info_len, unsigned int id, unsigned int begin, unsigned int end);
void mid_record_open1(int index);

/*
 自定义录制 打开
 其中 info_buf，info_len, id 都是 mid_record_open传递下来的，如果ID不为0，自定义录制可查找原先录制ID是否存在，如果存在就追加录制。
 返回值是录制对象的handle，-1表示无效
 */
typedef int (*RecordCall_open)(char *info_buf, int info_len, unsigned int id);
/*
 自定义录制 数据push
 handle 是 RecordCall_open 返回的。
 返回值为实际写（或发送）成功的长度。不可恢复的错误返回 -1
 */
typedef int (*RecordCall_push)(int handle, char *buf, int len);
/*
 自定义录制 关闭
 */
typedef void (*RecordCall_close)(int handle);
/*
	自定义录制注册
 */
int mid_record_call(int index, RecordCall_open rcall_open, RecordCall_push rcall_push, RecordCall_close rcall_close);

/*
	关闭录制
	id: 为录制交叉增加的参数，对应的录制ID，为0表示关闭录制任务
 */
#define mid_record_close mid_record_close_v1
void mid_record_close(int index, unsigned int id);

/*
	探测下载速度
 */
void mid_record_check_bandwidth(int index);

/*
	设置录制结束时间
	v1 增加一个id参数
 */
#define mid_record_set_endtime mid_record_set_endtime_v1
void mid_record_set_endtime(int index, unsigned int id, unsigned int endtime);

/*
	删除已录制的节目
 */
#define mid_record_delete mid_record_delete_v1
void mid_record_delete(unsigned int id, PvrMsgCall call);

unsigned int mid_record_get_num(void);

/*
	size应等于sizeof(struct PVRInfo)
	info_buf，info_len 为开始录制时，mid_record_open 传入的、将要保存到录制节点里的自定义信息。
		info_len 的初值应为info_buf 缓冲区空间
		info_len 的返回值为实际填充到info_buf里数据的长度
		注意：info_len 的初值、也就info_buf缓冲区空间 小于实际存入自定义信息的长度时，info_len将会被置为 0
	v2 去掉参数 size_t size
 */
#define mid_record_get_info	mid_record_get_info_v2
int mid_record_get_info(int index, struct PVRInfo *info, char *info_buf, int* info_len);

/*
	保存录制用户信息
	注意：该ID必须为录制列表中已存在的录制记录，否则会报错
 */
int mid_record_user_write(unsigned int id, char *buf, int len);
/*
	读取录制记录相关信息
	如果用户信息确认未保存过，最好不要调用该接口，否则会出现一条错误打印
	size 指示buf缓冲长度
	返回值
		小于0表示读取错误
		大于或等于0为实际读取长度
 */
int mid_record_user_read(unsigned int id, char *buf, int size);

//带宽比特率
void mid_record_limit_bandwidth(int index, int bitrate);
/*
 percent 临时带宽百分比，percent为0表示千分之一
 second 临时码流持续时间
 */
void mid_record_adjust_bandwidth(int index, int percent, int second);

/*
	下载带宽，mid_record_open之前调用
 */
void mid_record_bitrate(int index, int bitrate);

void mid_record_set_tuner(int index, int tuner);
void mid_record_set_fcc(int index, int fcc_type);
void mid_record_set_igmp(int index, MultiPlayCall play_call, MultiStopCall stop_call, void* igmp, int igmp_size);

void mid_record_virtual(int flag);

#ifdef __cplusplus
}
#endif

#endif//__MID_RECORD_H__

