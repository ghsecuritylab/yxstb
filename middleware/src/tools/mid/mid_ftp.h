#ifndef __MID_FTPCLIENT_H__
#define __MID_FTPCLIENT_H__

typedef struct mid_ftp*	mid_ftp_t;

mid_ftp_t mid_ftp_create(void);
int mid_ftp_open(mid_ftp_t ftp, char *hostname, char *username, char *password);
int mid_ftp_close(mid_ftp_t ftp);
int mid_ftp_put_begin(mid_ftp_t ftp, char *filename);
int mid_ftp_put_data(mid_ftp_t ftp, char *buf, int size);
int mid_ftp_put_end(mid_ftp_t ftp);

int mid_ftp_post(char *hostname, char *username, char *password, char *content, int length);

int mid_ftp_get_begin(mid_ftp_t ftp, char *filename);
int mid_ftp_get_data(mid_ftp_t ftp, char *buf, int size);
int mid_ftp_get_data_to(mid_ftp_t ftp, char * buf, int size, int ms);
int mid_ftp_get_end(mid_ftp_t ftp);


#endif //__MID_FTPCLIENT__

