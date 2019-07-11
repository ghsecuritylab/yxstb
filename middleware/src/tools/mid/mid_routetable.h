#ifndef DHCP_DHCP_ROUTETABLE_H_
#define DHCP_DHCP_ROUTETABLE_H_

#include "config/pathConfig.h"

#define ROUTETABLE_PATH DEFAULT_TEMP_DATAPATH"/abroute.tbl"

typedef struct _dhcp_route_entry
{
    char destip[16];
    char netmask[16];
    char plane[16];
    struct _dhcp_route_entry *pnext;
}DHCP_ROUTE_ENTRY, *PDHCP_ROUTE_ENTRY;

typedef struct _dhcp_route_table
{
    char terminaltype[20];
    char softver[20];
    char dnsselect[2]; /* 'A' or 'B' */
    unsigned short vlanid;
    char routever[8];
    struct _dhcp_route_entry *pentry;
}DHCP_ROUTE_TABLE, *PDHCP_ROUTE_TABLE;

#ifdef __cplusplus
extern "C" {
#endif

int dhcp_route_get(unsigned int serverip, unsigned short serverport, unsigned int localip,
                   const unsigned char *lpMAC, const char *lpModelId,
                   const char *lpModelName, const char *lpSwVersion, const char *lpRouteVer,
                   const char *lpChipType, const char *lpCPUArch, char** ppRouteTable);

/*  dhcp_route_parse
    return: 2 - route table parsed ok
            1 - memory limited
            0 - route table empty
           -1 - content error
           -2 - parameters error
*/
int dhcp_route_parse(char *pRouteTable, int length, PDHCP_ROUTE_TABLE *ppTable);
int dhcp_route_destroy(PDHCP_ROUTE_TABLE pRouteTable);
int dhcp_route_show(PDHCP_ROUTE_TABLE pRouteTable);
int dhcp_route_addtable(PDHCP_ROUTE_TABLE pRouteTable, const char *ip_a, const char *ip_b);
int dhcp_route_loadtable(const char *pPath, char **ppTable);

int dhcp_line_check(unsigned int serverip, unsigned short serverport, unsigned int localip,
                   const unsigned char *lpMAC, const char *lpModelId,
                   const char *lpModelName, const char *lpSwVersion,
                   const char *lpChipType, const char *lpCPUArch, char** ppResult);

#ifdef __cplusplus
}
#endif

#endif /* DHCP_DHCP_ROUTETABLE_H_ */

