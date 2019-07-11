/*
 * filename  : sqm_types.h
 * create date: 2010.12.7
 * discription: This file only keep the defines used both in sqm.elf and
 *              sqm porting module;
 */

#ifndef __SQM_TYPES_H__
#define __SQM_TYPES_H__

#include "config/pathConfig.h"

#define MAX_MSG_LEN 8192
#define MID_TO_SQM  DEFAULT_PIPE_DATAPATH"/mid_to_sqm"
#define	TELNET_FIFO	 DEFAULT_PIPE_DATAPATH"/telnet_fifo"
#define	FIFO_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH )

#define NET_NAME_ETH0 "eth0"
#define NET_NAME_PPPOE0 "ppp0"
#define NET_NAME_ETH0_WI "rausb0"

typedef enum NET_NAME_TAG
{
	TYPE_ETH0=1,
	TYPE_PPPOE0=2,
	TYPE_ETH0_WI=3
}NET_NAME;

typedef enum SQM_STAT_TAG
{
	SQM_READY = 1,
	SQM_INIT_OK = 2,
	SQM_START_OK = 3,
	SQM_SETCHANNELINFO_OK = 4,
	SQM_STOP_OK = 5,

	SQM_ERROR = 0xff
}SQM_STATUS;

typedef enum SQM_CMD_TAG
{
	SQM_CMD_INIT = 0x80,
	SQM_CMD_START_MONITOR = 0x81,
	SQM_CMD_SETCHANNELINFO = 0x82,
	SQM_CMD_STOP_MONITOR = 0x83,

	SQM_CMD_UNKNOWN = 0xff
}SQM_CMD;


#endif                        // __SQM_TYPES_H__

