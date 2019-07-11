#ifndef __NetworkMacro__H_
#define __NetworkMacro__H_

#define FIFO_PATH_DIR   "."
#define FIFO_NAME_SIZE  128
#define FIFO_FILE_MODE  00777
#define FIFO_BUFFSIZE   4096

#define NETWORK_DIR "/var/misc"
#define DHCP_ROOT_DIR NETWORK_DIR"/dhcp"
#define PPPOE_ROOT_DIR NETWORK_DIR"/ppp"

#define IFACE_NAME_SIZE 64

#define NL_ERR_UNKNOW -1
#define NL_FLG_DOWN    0
#define NL_FLG_RUNNING 1

#define NETDEV_PATH "/proc/net/dev"
typedef struct NetDevStats {
    unsigned long rxBytes;
    unsigned long rxPackets;
    unsigned long rxErrors;
    unsigned long rxDrop;
    unsigned long txBytes;
    unsigned long txPackets;
    unsigned long txErrors;
    unsigned long txDrop;
}NetDevStats_t;

#endif
