#ifndef __MID_HTTP_H__
#define __MID_HTTP_H__

#define 	HTTP_ERROR_CONNECT		-1
#define		HTTP_ERROR_SENDDATA		-2
#define		HTTP_ERROR_READDATA		-3
#define		HTTP_ERROR_DATA			-4
#define		HTTP_ERROR_NET			-5
#define		HTTP_ERROR_MALLOC		-6
#define		HTTP_ERROR_TIMEOUT		-7
#define		HTTP_ERROR_SOCKET		-8
#define		HTTP_ERROR_WRITEDATA	-9
#define     HTTP_ERROR_NOT200       -10
#define		HTTP_OK_READDATA		0
#define		HTTP_OK_LOCATION		1
#define		HTTP_OK_EMPTY			2
#define		HTTP_OK_BREAK			3

#ifdef __cplusplus
extern "C" {
#endif


typedef void (* mid_http_f)(int result, char* buf, int len, int arg);

//total �ļ��ܳ��� �����Chunkģʽ��Chunkģʽ�������ǰ��֪���ܳ�����totalΪ0
//offset Ŀǰ��������������ļ��е�ƫ��
//buf Ŀǰ���ص����ݿ�
//len Ŀǰ���ص����ݿ鳤��
typedef int (* mid_write_f)(int arg, int total, int offset, char* buf, int len);
int mid_http_init(void);
int mid_http_call( const char* url, mid_http_f callback, int arg, char *buf, int len, char *ex);
int mid_http_simplecall(char* url, mid_http_f callback, int arg);
int mid_http_writecall(char* url, mid_http_f callback, int arg, mid_write_f writecall, char *ex);
int mid_http_offsetcall( const char* url, mid_http_f callback, int arg, mid_write_f writecall, int offset, char *ex);
int mid_http_break(mid_http_f callback, int arg);
int httpDefaultFunc(int type, char* buf, int len, int arg);

typedef void (* mid_http_info_f)(char *info_buf, int info_len);
void mid_http_infoCall(mid_http_info_f httpinfo);

//��Ӧʱ�䣬��λ����
typedef void (* mid_http_rrt_f)(int ms);
typedef void (* mid_http_fail_f)(char* url, int code);
void mid_http_statCall(mid_http_rrt_f httprrt, mid_http_fail_f httpfail);
/*����ʱ�����Ӧ�ó���logoͼƬ�������ʽ��logo*/
//int app_logo_file2file();

typedef enum tagfile_httpGet_type{
	FileHttpGet_eBootLogo,
} file_httpGet_type_e;
/*
 */
int file_http_get( char* uri, mid_http_f callback, int arg, char *buf_data, int buf_len, int app );

int ctc_http_send_GETmessage(const char *url, const char *User_Agent);

#ifdef __cplusplus
}
#endif

#endif
