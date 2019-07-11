
#ifndef __DVBS_PORT_H__
#define __DVBS_PORT_H__

#include "codec.h"

typedef void (*dvb_writepmt_f)(unsigned int magic, DvbCaParam_t param);
typedef void (*dvb_writedata_f)(unsigned int magic, unsigned char *buf, int len);

void dvbs_port_init(void);

#define dvbs_port_play	dvbs_port_play_v5
int dvbs_port_play(int pIndex, int tuner, char *url, dvb_writepmt_f dvb_writepmt, dvb_writedata_f writedata, unsigned int magic);
int dvbs_port_check(int pIndex);
void dvbs_port_stop(int pIndex);
#define dvbs_port_change	dvbs_port_change_v3
void dvbs_port_change(int pIndex, int type, int *del_pid, int del_num, int *add_pid, int add_num);

//非标准函数
int dvbs_port_get_num(void);
int dvbs_port_get_id(int pIndex, int* id);
void dvbs_port_scan(void);

int dvbs_port_record_open(int pIndex);
void dvbs_port_record_push(int pIndex, char* buf, int len);
void dvbs_port_record_close(int pIndex);

/*
	 1: 允许
	 0: 待查（正在检测）
	-1: 禁止
 */
int dvbs_port_record_check(int pIndex);

#endif//__DVBS_PORT_H__
