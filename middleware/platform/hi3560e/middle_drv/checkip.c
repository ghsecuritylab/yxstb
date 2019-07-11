#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <arpa/inet.h>

#include "checkip.h"

#define LOG_ERR	    "error"
#define LOG_INFO	"info"

#define LOG(level, str, args...) do { printf("%s, ", level); \
				printf(str, ## args); \
				printf("\n"); } while(0)


#define MAC_BCAST_ADDR		(unsigned char *) "\xff\xff\xff\xff\xff\xff"

static struct ip_config our_config = 
{
	.ourmac = {0},
	.ourip = 0,
	.interface = NULL
};

static int read_interface(const char *interface, u_int32_t *addr, unsigned char *mac)
{
	int fd;
	struct ifreq ifr;
	struct sockaddr_in *our_ip;

	memset(&ifr, 0, sizeof(struct ifreq));
	if((fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) 
	{
		LOG(LOG_ERR, "socket failed!: %s", strerror(errno));
		return -1;
	}

	ifr.ifr_addr.sa_family = AF_INET;
	strcpy(ifr.ifr_name, interface);

	if (ioctl(fd, SIOCGIFADDR, &ifr) == 0) 
	{
		our_ip = (struct sockaddr_in *) &ifr.ifr_addr;
		*addr = our_ip->sin_addr.s_addr;
		//LOG(LOG_INFO, "%s (our ip) = %s %d", ifr.ifr_name, inet_ntoa(our_ip->sin_addr), *addr);
	}
	else 
	{
		LOG(LOG_ERR, "SIOCGIFADDR failed, is the interface up and configured?: %s", strerror(errno));
		close(fd);
		return -1;
	}
	
	if (ioctl(fd, SIOCGIFHWADDR, &ifr) == 0) 
	{
		memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);
		//LOG(LOG_INFO, "adapter hardware address %02x:%02x:%02x:%02x:%02x:%02x",	mac[0], mac[1], mac[2], mac[3], mac[4], mac	[5]);
	} 
	else 
	{
		LOG(LOG_ERR, "SIOCGIFHWADDR failed!: %s", strerror(errno));
		close(fd);
		return -1;
	}
	
	close(fd);
	return 0;
}

static int check_ip( const char *interface, u_int32_t ip, unsigned char *mac )
{
	int optval = 1;
	int	s;			/* socket */
	struct sockaddr addr;		/* for interface name */
	struct arpMsg	arp;
	fd_set		fdset;
	struct timeval	tm={1,0};

	if ((s = socket (PF_PACKET, SOCK_PACKET, htons(ETH_P_ARP))) == -1) 
	{
		LOG(LOG_ERR, "Could not open raw socket");
		return 0;
	}
	
	if (setsockopt(s, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval)) == -1) 
	{
		LOG(LOG_ERR, "Could not setsocketopt on raw socket");
		close(s);
		return 0;
	}
	
	/* send arp request */
	memset(&arp, 0, sizeof(arp));
	memcpy(arp.ethhdr.h_dest, MAC_BCAST_ADDR, 6);	/* MAC DA */
	memcpy(arp.ethhdr.h_source, mac, 6);			/* MAC SA */
	arp.ethhdr.h_proto = htons(ETH_P_ARP);			/* protocol type (Ethernet) */
	arp.htype = htons(ARPHRD_ETHER);				/* hardware type */
	arp.ptype = htons(ETH_P_IP);					/* protocol type (ARP message) */
	arp.hlen = 6;									/* hardware address length */
	arp.plen = 4;									/* protocol address length */
	arp.operation = htons(ARPOP_REQUEST);			/* ARP op code */
	
	memcpy(arp.sHaddr, mac, 6);						/* source hardware address */
	memcpy(arp.sInaddr, &ip, 4);					/* source IP address */
	arp.sInaddr[3] = 0;
	memcpy(arp.tInaddr, &ip, 4);					/* target IP address */
	//printf("***********%d.%d.%d.%d\n",arp.sInaddr[0],arp.sInaddr[1],arp.sInaddr[2],arp.sInaddr[3]);
	memset(&addr, 0, sizeof(addr));
	strcpy(addr.sa_data, interface);
	if (sendto(s, &arp, sizeof(arp), 0, &addr, sizeof(addr)) < 0){
		close(s);
		return 0;
	}
	
	/* wait arp reply, and check it */
	FD_ZERO(&fdset);
	FD_SET(s, &fdset);
	
	if (select(s + 1, &fdset, (fd_set *) NULL, (fd_set *) NULL, &tm) < 0) 
	{
		LOG(LOG_ERR, "Error on ARPING request: %s", strerror(errno));
		close(s);
		return 0;
	} 
	else if (FD_ISSET(s, &fdset)) 
	{
		if (recv(s, &arp, sizeof(arp), 0) < 0 ){
			close(s);
			return 0;
		}
			
		if (bcmp(arp.sInaddr, &ip, 4) == 0) 
		{
			close(s);
			return -1;
		}
	}
	
	close(s);
	return 0;
}

int yx_ip_conflict_detect( const char *interface )
{
	if( interface==NULL || strlen(interface)==0 )
		return 0;
		
	if( our_config.interface == NULL )
		our_config.interface = interface;
	if( strcmp(our_config.interface,interface) || our_config.ourip==0 )
	{
		our_config.interface = interface;
		if (read_interface(our_config.interface, &our_config.ourip, our_config.ourmac) < 0)
		{
			LOG(LOG_ERR, "read our ip and mac fail!");
			return 0;
		}
	}
	
	return check_ip( our_config.interface, our_config.ourip, our_config.ourmac );
}



