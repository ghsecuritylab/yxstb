
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <unistd.h>

#include <sys/time.h>
#include <sys/mount.h>
#include <asm/page.h> 
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>

#include <sys/select.h>
#include <time.h>
#include <linux/input.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <errno.h> 
#include <stdint.h> 
#include <stdarg.h>

//由于编译器的替换找不见/usr/include/asm/socket.h头文件,故采用
#define SO_RCVBUFFORCE 33

static int g_isDeviceReady = 0;

pthread_t g_hotplugThd;
int g__hotplug_sock = -1;
char g_mountPath[64];

int open_hotplug_sock(void)
{
    struct sockaddr_nl snl;
    const int buffersize = 16 * 1024;
    int retval;
 
    memset(&snl, 0x00, sizeof(struct sockaddr_nl));
    snl.nl_family = AF_NETLINK;
    snl.nl_pid = getpid();
    snl.nl_groups = 1;
 
    g__hotplug_sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
    if ( g__hotplug_sock == -1) {
        printf("error getting socket: %s", strerror(errno));
        return -1;
    }
 
    /* set receive buffersize */
    setsockopt( g__hotplug_sock, SOL_SOCKET, SO_RCVBUFFORCE, &buffersize, sizeof(buffersize) );
 
    retval = bind( g__hotplug_sock, (struct sockaddr *) &snl, sizeof(struct sockaddr_nl) );
    if (retval < 0) {
        printf("bind failed: %s", strerror(errno));
        close( g__hotplug_sock );
        g__hotplug_sock = -1;
        return -1;
    }
 
    return g__hotplug_sock;
}

static const char * getMountPath( const char* hostname  )
{
	snprintf( g_mountPath, 63, "/dev/scsi/%s/bus0/target0/lun0/part1", hostname );

	return g_mountPath;
}

static int usb_try_mount( const char* pathname  )
{
	char cmdLine[64];
	
	memset( cmdLine, 0, 64 );
	snprintf( cmdLine, 63, "mount -t vfat %s /mnt", pathname );
	
	system( cmdLine );

	usleep(5000);
	
	return 0;
}

int usb__try_umount( void )
{
	 system("umount /mnt");
	 return 0;	
}

char g_hostname[16] = {0};
int hotplug__try_mount_usb( int hotplug_fd )
{
	//在这个里面增加对usb热插拔的监听
	int l_ret;
	char *lp_tmp = NULL;
	const char *pathname = NULL;
	char buf[4096];

	
	l_ret = recv( hotplug_fd, &buf, sizeof(buf), 0 ); 
	
	/* 如果不加下面这行打印, 串口会出现不输出任何打印, 必须按回车才打印几个字符的问题 */
	fprintf(stderr, "Hotplug: %s\n", buf);
	if ( l_ret <= 0)
		return 0;
	
	if( strstr(buf, "add@") && strstr(buf, "/class/scsi_host/host") ){
		lp_tmp = rindex( buf, '/');
		strncpy( g_hostname, lp_tmp + 1, 15);
	}
	
	if( strstr(buf, "umount@") && strstr(buf, "/block/sd") ){
		g_isDeviceReady = 0;
	}
	
	if( strstr(buf, "add@") && strstr(buf, "/block/sd") ){
		fprintf(stderr, "------add message---------\n");
		
		if( ( NULL != strstr(buf, "/block/sda/sda") ) || ( NULL != strstr(buf, "/block/sdb/sdb") ) ){
			fprintf(stderr, "------add message---------\n");
			pathname = getMountPath( g_hostname  );
			if( NULL == pathname ){
				return 0;
			}
			
			usb_try_mount( pathname  );
			
			g_isDeviceReady = 1;
			return 1;
		}
	}
	
	if( strstr(buf, "remove@") && strstr(buf, "/block/sd") ){
		fprintf(stderr, "------remove message---------\n");
		if( ( NULL != strstr(buf, "/block/sda/sda") ) || ( NULL != strstr(buf, "/block/sdb/sdb") ) ){
			return 2;
		}
		
	}
	
	return 0;	
}


