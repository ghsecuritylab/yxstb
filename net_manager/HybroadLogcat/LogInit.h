#ifndef _LOG_INIT_H_
#define _LOG_INIT_H_

int initLog();
void *attachUdpLogFilter(char *server_ip, int port, int log_type, int log_level); //server format:ip:port
int detachUdpLogFilter(void *filter);

#endif
