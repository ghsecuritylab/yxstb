/***************************************************************************
 *     Copyright (c) 2007-2008, Yuxing Software Corporation
 *     All Rights Reserved
 *     Confidential Property of Yuxing Softwate Corporation
 * $Create_Date: 2008-4-23 16:34 $
 * Revision History:
 * 1. by SunnyLi  2008-4-23 16:34 create

 * $contact at lizhaohui@yu-xing.com
 * this is network interface drv for YX5331A from linux drv system net
 ***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <pthread.h>
#include <signal.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <yx_debug.h>

#include <sys/stat.h>
#include <errno.h>
#include <net/if.h>

#include <linux/sockios.h>
#include "ethtool-util.h"

#ifndef SIOCETHTOOL
#define SIOCETHTOOL     0x8946
#endif
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

#include "yx_api.h"

#define SIOCGLINKSTATE      (SIOCDEVPRIVATE + 0)

int yxprint_flag =1;

#define MDIO_REG_BASE   0x90030000
#define MDIO_REG_LEN    0x200
#define MDIO_REG_REG3   0x18C
#define MEM_DEV         "/dev/mem"


#if 0 //removed by teddy at 2011-2-17 17:18:51
YX_S32 yx_middle_drv_net_led_control(unsigned int onoff)
{
	int ret =0;
	unsigned int net_state = 1;
	if(!onoff)
	    net_state = 0;
	else
	    net_state = 1;

	ret = yx_middle_drv_gpio_write(43,&net_state);

	return ret;
}
#endif
YX_S32 yx_middle_drv_net_get_link_status(char* devname, int *status)
{
	struct ifreq ifr;
	int fd;
	int err;
	struct ethtool_value edata;
	if(!devname)
		return -1;
	//char devname[] = "eth0";

	/* Setup our control structures. */
	memset(&ifr, 0, sizeof(ifr));

	/* Open control socket. */
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		perror("Cannot get control socket");
		return -1;
	}

	*status = 0;
	edata.cmd = ETHTOOL_GLINK;
	ifr.ifr_data = (caddr_t)&edata;
	strcpy( ifr.ifr_name, devname);
	err = ioctl(fd, SIOCETHTOOL, &ifr);
	if (err == 0) {
		if( edata.data )
			*status = 1;
	} else if (errno != EOPNOTSUPP) {
		perror("Cannot get link status");
	}
	close(fd);

	return YX_RET_SUCCESS;
}

YX_S32 yx_middle_net_get_ipaddr(char *if_dev_name,char *ip,char *subnet_mask)
{
	struct ifreq ifr;
	int so = -1;

	if (NULL == if_dev_name)
		return YX_FAILURE;
	if ((so = socket(PF_PACKET, SOCK_RAW, 0)) < 0){
		return YX_FAILURE;
	}
	strcpy(ifr.ifr_name,if_dev_name);

	if (ip){
		bzero (&(ifr.ifr_addr), sizeof (struct sockaddr_in));
		((struct sockaddr_in *) &ifr.ifr_addr)->sin_family = AF_INET;
		if (ioctl(so, SIOCGIFADDR, &ifr) < 0){
			close(so);
			return YX_FAILURE;
		}
		strcpy(ip,inet_ntoa(((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr));
	}
	if (subnet_mask){
		bzero (&(ifr.ifr_netmask), sizeof (struct sockaddr_in));
		((struct sockaddr_in *) &ifr.ifr_netmask)->sin_family = AF_INET;
		if (ioctl(so, SIOCGIFNETMASK, &ifr) < 0){
			close(so);
			return YX_FAILURE;
		}
		strcpy(subnet_mask,inet_ntoa(((struct sockaddr_in *) &ifr.ifr_netmask)->sin_addr));
	}
	close(so);
	return YX_SUCCESS;
}

YX_S32 yx_middle_net_set_ipaddr(char *if_dev_name,char *ip,char *subnet_mask)
{
	struct ifreq ifr;
	int so = -1;

	if ( NULL == if_dev_name)
		return YX_FAILURE;
	if ((so = socket(PF_PACKET, SOCK_RAW, 0)) < 0)
		return YX_FAILURE;
	strcpy(ifr.ifr_name,if_dev_name);

	if (ip){
		bzero (&(ifr.ifr_addr), sizeof (struct sockaddr_in));
		((struct sockaddr_in *) &ifr.ifr_addr)->sin_family = AF_INET;
		((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr.s_addr = inet_addr (ip);
		if(ioctl(so,SIOCSIFADDR,&ifr) < 0){
			close(so);
			return YX_FAILURE;
		}
		ifr.ifr_ifru.ifru_flags = ifr.ifr_ifru.ifru_flags | IFF_UP;
		if (ioctl(so, SIOCSIFFLAGS, &ifr) < 0){
			close(so);
			return YX_FAILURE;
		}
	}
	if (subnet_mask){
		bzero (&ifr.ifr_netmask, sizeof (ifr.ifr_netmask));
		((struct sockaddr_in *) &ifr.ifr_netmask)->sin_family = AF_INET;
		((struct sockaddr_in *) &ifr.ifr_netmask)->sin_addr.s_addr = inet_addr(subnet_mask);
		if (ioctl(so, SIOCSIFNETMASK, &ifr) < 0){
			close(so);
			return YX_FAILURE;
		}
	}
	close(so);
	return YX_SUCCESS;
}


