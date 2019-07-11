/*******************************************************************************
	公司：
			Yuxing software
	纪录：
			2008-1-26 21:12:26 create by Liu Jianhua
	模块：
			tr069
	简述：
			HTTP 协议
 *******************************************************************************/

#ifndef __TR069_HTTP_H__
#define __TR069_HTTP_H__

#define HTTP_HOST_LEN		64
#define HTTP_URI_LEN		256
#define HTTP_HEAD_LEN		(1024 * 2)

enum {
	AUTHENTICATION_NONE = 0,
	AUTHENTICATION_BASIC,
	AUTHENTICATION_DIGEST
};

#define HTTP_ERROR_CONNNECT	-2

#define DIGEST_METHOD_LEN	16
#define DIGEST_MD5_LEN32	32
#define DIGEST_PARAM_LEN	64
#define DIGEST_NAME_LEN		24
#define DIGEST_RESPONSE_LEN	192

struct Digest {
	char method[DIGEST_METHOD_LEN];
	char uri[HTTP_URI_LEN];
	char username[DIGEST_PARAM_LEN];
	char password[DIGEST_PARAM_LEN];

	u_int nc;
	char cnonce[DIGEST_PARAM_LEN];

	char md5body[DIGEST_MD5_LEN32 + 4];

	char realm[DIGEST_PARAM_LEN];
	char nonce[DIGEST_PARAM_LEN];
	char opaque[DIGEST_PARAM_LEN];
	char algorithm[DIGEST_PARAM_LEN];//"md5-sess" or "md5"
	char qop[DIGEST_PARAM_LEN];

	char response[DIGEST_RESPONSE_LEN];
};

#define COOKIE_ITEM_NUM		8
#define COOKIE_NAME_LEN		32
#define COOKIE_VALUE_LEN	128

struct Cookie {
	char name[COOKIE_NAME_LEN];
	char value[COOKIE_VALUE_LEN];
};

struct Http {
	SOCKET sock;
	int code;
	char buf[HTTP_HEAD_LEN + 4];
	char header[HTTP_HEAD_LEN];
	int authen;
	int discnnct;
	char host[HTTP_HOST_LEN + 4];
	unsigned short port;
	unsigned short reserve_align;

	struct Digest digest;

	int cookie_num;
	struct Cookie cookie_array[COOKIE_ITEM_NUM];
};

void tr069_http_init(struct Http *http);

int tr069_http_digest(struct Http *http, const char *header);

int tr069_http_connect(struct Http *http);
int tr069_http_disconnect(struct Http *http);
int tr069_http_transfer(struct Http *http, char *content, int *pLength, int size);

int tr069_http_accept(struct Http *http, SOCKET srvsock,  char* buffer, int* pLength);
int tr069_http_post(char *url, char *username, char *password, char *content, int length);

int tr069_ftp_put(char *url, char *username, char *password, char *content, int length);
int tr069_ftp_get(char *url, char *username, char *password, char *content, int length);

#endif //__TR069_HTTP_H__

