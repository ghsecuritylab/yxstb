#ifndef NM_DBG_H
#define NM_DBG_H
/*
1、调整media_dbg.c中的OUTPUT_LEVEL的值可以控制信息的输出级别。此时影响的将是全部工程内的所有打印信息
2、在某个文件中，控制某些打印输出则可以在文件中设置宏。然后修改宏的值控制某些打印的输出。
*/

#define LOG_NONE      -1
#define LOG_DEBUG      0
#define LOG_WARNING 1
#define LOG_ERROR      2

#ifdef __cplusplus
extern "C"{
#endif

int file_dump(char *file_name, char *buf, int buf_len);
void nm_dbg(int log_level, const char *file, const char *function, int line, const char *fmt, ...);
int nm_dbg_sysinfo_set(char *version, char *additional_version, char *mac);
//void nm_dbg(int log_level, const char *fmt, ...);
#define nm_track()  nm_dbg(LOG_DEBUG, __FILE__, __FUNCTION__, __LINE__, NULL)
#define nm_track_level(x)  nm_dbg(x,  __FILE__, __FUNCTION__, __LINE__, NULL)
#define nm_msg(string, args... )  nm_dbg(LOG_ERROR,  __FILE__, __FUNCTION__, __LINE__, string, ##args )
#define nm_msg_level(x, string, args... )   nm_dbg(x, __FILE__, __FUNCTION__, __LINE__, string, ##args )

#ifdef __cplusplus
};
#endif


#endif

