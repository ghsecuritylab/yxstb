
#ifndef _SQAPUBLIC_H_
#define _SQAPUBLIC_H_

unsigned int SqaGetMS(void);
unsigned short SqaGetPort(void);

void sqaRecvData(int fd, STB_HANDLE handle, funFccRecPacket *fun, struct RTSP* rtsp,
    fd_writedata_f fd_writedata, fd_writeable_f fd_writeable);
void SqaHandleEvent(STB_HANDLE handle, struct RTSP* rtsp, 
    fd_writedata_f fd_writedata, fd_writeable_f fd_writeable);

int channel_array_get_server(unsigned int mult, unsigned int port, unsigned int *fcc_ip, int *flag);
void channel_array_set_fcc_validtime(unsigned int mult, unsigned int port, unsigned int validTime);
int channel_array_update_fcc_server_addr(unsigned int mult, unsigned int port, unsigned int fcc_ip);

#endif
