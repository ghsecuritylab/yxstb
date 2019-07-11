
#ifndef __TR069_ALARM_V2_H__
#define __TR069_ALARM_V2_H__

enum {
	ALARM_TYPE_STB_Upgrade = 0,//1. 机顶盒升级失败
	ALARM_TYPE_Disk_Write,//2. 磁盘读写失败
	ALARM_TYPE_CPU_Used,//3. CPU使用百分比超过阈值
	ALARM_TYPE_Disk_Used,//4. 磁盘使用空间百分比超过阈值
	ALARM_TYPE_Memory_Used,//5. 内存使用百分比超过阈值
	ALARM_TYPE_Dropped,//6. 丢包率超过阈值
	ALARM_TYPE_Authorize,//7. IPTV业务认证失败
	ALARM_TYPE_Channel,//8. 加入频道失败告警
	ALARM_TYPE_Decode,//9. 机顶盒解码错误

    ALARM_TYPE_Decrypt,//10.机顶盒解密失败
    ALARM_TYPE_Dropframe,//11.丢帧率超过阈值
    ALARM_TYPE_Timelapse,//12.时延超过阈值
    ALARM_TYPE_Cushion,      //13.缓冲溢出(上溢/下溢)
    ALARM_TYPE_EPG_Access, //14.连接EPG 服务器失败
    ALARM_TYPE_Media_Access, //15.访问媒体服务器失败
    ALARM_TYPE_File_Access,  //16.文件服务器连接失败
    ALARM_TYPE_Media_Format, //17.媒体格式不支持

	ALARM_TYPE_MAX
};

enum {
	ALARM_LEVEL_NONE = 0,
	ALARM_LEVEL_CRITICAL,//紧急
	ALARM_LEVEL_MAJOR,//重要
	ALARM_LEVEL_MINOR,//次要
	ALARM_LEVEL_WARNING,//提示
	ALARM_LEVEL_INDETERMINATE,//不确定
	ALARM_LEVEL_CLEARED,
	ALARM_LEVEL_MAX
};

#ifdef __cplusplus
extern "C" {
#endif

int tr069_port_alarm_post(int type, char *code, int level, char *location);
void tr069_port_alarm_clear(int alarm);

#ifdef __cplusplus
}
#endif

#endif //__TR069_ALARM_H__
