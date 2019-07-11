
#ifdef INCLUDE_DLNA

#include <stdio.h>
#include <unistd.h>
#include "semaphore.h"

#include "sys_msg.h"
#include "sys_basic_macro.h"
#include "json/json.h"

#include "mid_dlna_ex.h"
#include "hitTime.h"

#include "upnp/LinkedList.h"
#include "dlna_api.h"

#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include "libzebra.h"
#ifdef ENABLE_DLNA_TEST

#define WAIT_AND_PRINT()	{/*while(1)*/{printf("i am at dlna-debugging mode.\n%s %d\n"__FILE__,__LINE__);sleep(10);}}
int mid_dlna_start_ex(int argc, char* argv[])
{
#if(DLNA_TEST_MODE==DLNA_TEST_OVERSEA)
	char *input_url = argv[1];
	usleep(40*1000);
	mid_dlna_start(-1);
	usleep(400*1000);

#elif(DLNA_TEST_MODE==DLNA_TEST_SINGLE_FUNC)
	WAIT_AND_PRINT();

#elif(DLNA_TEST_MODE==DLNA_TEST_HW_JS)
	mid_dlna_start(-1);
	WAIT_AND_PRINT();

#else

#endif
	return 0;
}

/*-----------------------------------------------------------------------------------------*/
/* test yuxing interface */
/*-----------------------------------------------------------------------------------------*/
#define MAX_INTERFACES 256
#define DEFAULT_INTERFACE 1
#define UPNP_E_INIT -1
int mid_dlna_getlocalhostname( char *out )
{
    char szBuffer[MAX_INTERFACES * sizeof( struct ifreq )];
    struct ifconf ifConf;
    struct ifreq ifReq;
    int nResult;
    int i;
    int LocalSock;
    struct sockaddr_in LocalAddr;
    int j = 0;

    // Create an unbound datagram socket to do the SIOCGIFADDR ioctl on.
    if( ( LocalSock = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP ) ) < 0 ) {
        return UPNP_E_INIT;
    }
    // Get the interface configuration information...
    ifConf.ifc_len = sizeof szBuffer;
    ifConf.ifc_ifcu.ifcu_buf = ( caddr_t ) szBuffer;
    nResult = ioctl( LocalSock, SIOCGIFCONF, &ifConf );
    if( nResult < 0 ) {
        return UPNP_E_INIT;
    }

    // Cycle through the list of interfaces looking for IP addresses.
    for( i = 0; ( ( i < ifConf.ifc_len ) && ( j < DEFAULT_INTERFACE ) ); ) {
        struct ifreq *pifReq =
            ( struct ifreq * )( ( caddr_t ) ifConf.ifc_req + i );
        i += sizeof *pifReq;

        // See if this is the sort of interface we want to deal with.
        strcpy( ifReq.ifr_name, pifReq->ifr_name );
        if( ioctl( LocalSock, SIOCGIFFLAGS, &ifReq ) < 0 ) {
        }
        // Skip loopback, point-to-point and down interfaces,
        // except don't skip down interfaces
        // if we're trying to get a list of configurable interfaces.
        if( ( ifReq.ifr_flags & IFF_LOOPBACK )
            || ( !( ifReq.ifr_flags & IFF_UP ) ) ) {
            continue;
        }
        if( pifReq->ifr_addr.sa_family == AF_INET ) {
            // Get a pointer to the address...
            memcpy( &LocalAddr, &pifReq->ifr_addr,
                    sizeof pifReq->ifr_addr );

            // We don't want the loopback interface.
            if( LocalAddr.sin_addr.s_addr == htonl( INADDR_LOOPBACK ) ) {
                continue;
            }

        }
        //increment j if we found an address which is not loopback
        //and is up
        j++;

    }
    close( LocalSock );

    strcpy( out, inet_ntoa( LocalAddr.sin_addr ));

    return 0;

}
extern int g_dlna_running_flag;
static char *dlna_js_identifier = "DLNA.";
int mid_dlna_JsRead(const char * params,char * buf,int length)
{
	if( strncasecmp(params, dlna_js_identifier, strlen(dlna_js_identifier))  )
		return -1;
	while(g_dlna_running_flag==0) usleep(1000*10);
	return Dlna_JseRead(params, buf, length);
}
int mid_dlna_JsWrite(const char * params,char * buf,int length)
{
	if( strncasecmp(params, dlna_js_identifier, strlen(dlna_js_identifier))  )
		return -1;
	while(g_dlna_running_flag==0) usleep(1000*10);
	return Dlna_JseWrite(params, buf, length);
}


/*-----------------------------------------------------------------------------------------*/
/* test huawei js */
/*-----------------------------------------------------------------------------------------*/
extern int Dmc_HuaweiJse_Read(const char *func, const char *para, char *value, int len);
extern int Dmc_HuaweiJse_Write(const char *func, const char *para, char *value, int len);
extern int Dmp_HuaweiJse_Read(const char *func, const char *para, char *value, int len);
extern int Dmp_HuaweiJse_Write(const char *func, const char *para, char *value, int len);
extern int g_dlna_homenas_found;
int mid_dlna_test(unsigned int msgno, int type, int stat)
{
    int len=4096, ret;
	int num=0, i=0;

	char *func, *para = NULL, value[4096], temp[1024];
    char *udn = "uuid:898f9738-d930-4db4-a3cf-0007679bed36";
	char *objectID = NULL, *classID = NULL, *itemType = NULL;

	struct json_object *ret_json, *ret_num;

//    if(!g_dlna_homenas_found)
//        return -1;
	if(YX_EVENT_KEYDOWN != type || YX_FP_POWER != msgno)
		return -1;

#if 0
/* ItemNumber_Get::deviceID,classID
{"classID":11,"items_num":"56","deviceID":" deviceUDN"}*/
	func = "ItemNumber_Get";
	classID  = "100";
	sprintf(temp, "%s,%s", udn, classID);
	value[0]=0;
	ret = Dmc_HuaweiJse_Read(func, temp, value, len);
	printf("----test: %s\n\r", value);

	ret_json = dlna_json_tokener_parse(value);
	ret_num = json_object_object_get(ret_json, "items_num");
	num = json_object_get_int(ret_num);
//	for(i = 0; i < num; )
	{
		/* ItemList_Get:: deviceID,classID,postion,count */
		func = "ItemList_Get";
		sprintf(temp, "%s,%s,%d,%d", udn, classID, i, i+5);
		value[0]=0;
		//sJson=iPanel.ioctlRead("ContItemList_Get: deviceID ,containerID,postion,count,classID,itemType")
		ret = Dmc_HuaweiJse_Read(func, temp, value, len);
		i += 5;
	}
	json_object_put(ret_json);
#endif

#if 0
	udn = "uuid:898f9738-d930-4db4-a3cf-0007673ad5a6";

	func = "ContItemNumber_Get";
	objectID = "1";
	classID  = "3301";
	itemType = "2";
	sprintf(temp, "%s,%s,%s,%s", udn, objectID, classID, itemType);
	value[0]=0;
	ret = Dmc_HuaweiJse_Read(func, temp, value, len);
	printf("----pvr: %s\n\r", value);

	ret_json = dlna_json_tokener_parse(value);
	if(ret_json)
	{
		ret_num = json_object_object_get(ret_json, "items_num");
		num = json_object_get_int(ret_num);
		for(i = 0; i < num; )
		{
			func = "ContItemList_Get";
			sprintf(temp, "%s,%s,%d,%d,%s,%s", udn, objectID, i, i+5, classID, itemType);
			value[0]=0;
			//sJson=iPanel.ioctlRead("ContItemList_Get: deviceID ,containerID,postion,count,classID,itemType")
			ret = Dmc_HuaweiJse_Read(func, temp, value, len);
			i += 5;
		}
		json_object_put(ret_json);
	}
#endif
#if 0
/* sValue=iPanel.ioctlRead('DeviceNumber_Get,{"deviceType":"deviceType"}') */
	func = "DeviceNumber_Get";
	temp[0] = 0;
	value[0]=0;
	ret = Dmc_HuaweiJse_Read(func, temp, value, len);
	printf("----test: %s\n\r", value);

	func = "DeviceNumber_Get";
	sprintf(temp, "{\"deviceType\":\"ALL\"}");
	value[0]=0;
	ret = Dmc_HuaweiJse_Read(func, temp, value, len);
	printf("----test: %s\n\r", value);

	func = "DeviceList_Get";
	sprintf(temp, "%d,%d", 0, 0);
	value[0]=0;
	ret = Dmc_HuaweiJse_Read(func, temp, value, len);
	printf("----test: %s\n\r", value);

	func = "DeviceNumber_Get";
	sprintf(temp, "{\"deviceType\":\"STB\"}");
	value[0]=0;
	ret = Dmc_HuaweiJse_Read(func, temp, value, len);
	printf("----test: %s\n\r", value);

	func = "DeviceList_Get";
	sprintf(temp, "%d,%d", 0, 0);
	value[0]=0;
	ret = Dmc_HuaweiJse_Read(func, temp, value, len);
	printf("----test: %s\n\r", value);

#if 0
/*test network accident*/

	udn = "uuid:898f9738-d930-4db4-a3cf-00076739c2d0";
	func = "ContItemNumber_Get";
	objectID = "0";
	classID  = "1";

	{
		sprintf(temp, "%s,%s,%s, ", udn, objectID, classID);
		value[0]=0;
		ret = Dmc_HuaweiJse_Read(func, temp, value, len);
		printf("----test: %s\n\r", value);
		ret_json = dlna_json_tokener_parse(value);

		itemType = "1";
		ret_num = json_object_object_get(ret_json, "containers_num");
		num = json_object_get_int(ret_num);
		//sJson=iPanel.ioctlRead("ContItemList_Get: deviceID ,containerID,postion,count,classID,itemType")
		for(i = 0; i < num; )
		{
			func = "ContItemList_Get";
			sprintf(temp, "%s,%s,%d,%d,%s,%s", udn, objectID, i, i+5, classID, itemType);
			value[0]=0;
			ret = Dmc_HuaweiJse_Read(func, temp, value, len);
			i += 5;
		}

		itemType = "2";
		ret_num = json_object_object_get(ret_json, "items_num");
		num = json_object_get_int(ret_num);
		//sJson=iPanel.ioctlRead("ContItemList_Get: deviceID ,containerID,postion,count,classID,itemType")
		for(i = 0; i < num; )
		{
			func = "ContItemList_Get";
			sprintf(temp, "%s,%s,%d,%d,%s,%s", udn, objectID, i, i+5, classID, itemType);
			value[0]=0;
			ret = Dmc_HuaweiJse_Read(func, temp, value, len);
			i += 5;
		}
		json_object_put(ret_json);
	}
#endif

#if 0
/* test search */
/* ItemNumber_Get::deviceID,classID
{"classID":11,"items_num":"56","deviceID":" deviceUDN"}*/
	func = "ItemNumber_Get";
	classID  = "1";
	sprintf(temp, "%s,%s", udn, classID);
	value[0]=0;
	ret = Dmc_HuaweiJse_Read(func, temp, value, len);
	printf("----test: %s\n\r", value);

	ret_json = dlna_json_tokener_parse(value);
	ret_num = json_object_object_get(ret_json, "items_num");
	num = json_object_get_int(ret_num);
//	for(i = 0; i < num; )
	{
		/* ItemList_Get:: deviceID,classID,postion,count */
		func = "ItemList_Get";
		sprintf(temp, "%s,%s,%d,%d", udn, classID, i, i+5);
		value[0]=0;
		//sJson=iPanel.ioctlRead("ContItemList_Get: deviceID ,containerID,postion,count,classID,itemType")
		ret = Dmc_HuaweiJse_Read(func, temp, value, len);
		i += 5;
	}
	json_object_put(ret_json);
#else
/*test pvr only*/
	func = "ContItemNumber_Get";
	objectID = "1";
	classID  = "3301";
	itemType = "2";
	char *lang = "hun";
	sprintf(temp, "%s,%s,%s,%s,%s", udn, objectID, classID, itemType, lang);
	value[0]=0;
	ret = Dmc_HuaweiJse_Read(func, temp, value, len);
	printf("----pvr: %s\n\r", value);

	ret_json = dlna_json_tokener_parse(value);
	ret_num = json_object_object_get(ret_json, "items_num");
	num = json_object_get_int(ret_num);
	for(i = 0; i < num; )
	{
		func = "ContItemList_Get";
		sprintf(temp, "%s,%s,%d,%d,%s,%s", udn, objectID, i, i+5, classID, itemType);
		value[0]=0;
		//sJson=iPanel.ioctlRead("ContItemList_Get: deviceID ,containerID,postion,count,classID,itemType")
		ret = Dmc_HuaweiJse_Read(func, temp, value, len);
		i += 5;
	}
	json_object_put(ret_json);

/*test vod only*/
	func = "ContItemNumber_Get";
	objectID = "1";
	classID  = "3302";
	itemType = "2";
	sprintf(temp, "%s,%s,%s,%s", udn, objectID, classID, itemType);
	value[0]=0;
	ret = Dmc_HuaweiJse_Read(func, temp, value, len);
	printf("----vod: %s\n\r", value);

	ret_json = dlna_json_tokener_parse(value);
	ret_num = json_object_object_get(ret_json, "items_num");
	num = json_object_get_int(ret_num);
	for(i = 0; i < num; )
	{
		func = "ContItemList_Get";
		sprintf(temp, "%s,%s,%d,%d,%s,%s", udn, objectID, i, i+5, classID, itemType);
		value[0]=0;
		//sJson=iPanel.ioctlRead("ContItemList_Get: deviceID ,containerID,postion,count,classID,itemType")
		ret = Dmc_HuaweiJse_Read(func, temp, value, len);
		i += 5;
	}
	json_object_put(ret_json);


/* test image in root folder */
	func = "ContItemNumber_Get";
	objectID = "2";
	classID  = "1";
	itemType = "1";
	sprintf(temp, "%s,%s,%s,%s", udn, objectID, classID, itemType);
	value[0]=0;
	ret = Dmc_HuaweiJse_Read(func, temp, value, len);
	printf("----image: %s\n\r", value);

	ret_json = dlna_json_tokener_parse(value);
	ret_num = json_object_object_get(ret_json, "items_num");
	num = json_object_get_int(ret_num);
	for(i = 0; i < num; )
	{
		func = "ContItemList_Get";
		sprintf(temp, "%s,%s,%d,%d,%s,%s", udn, objectID, i, i+5, classID, itemType);
		value[0]=0;
		//sJson=iPanel.ioctlRead("ContItemList_Get: deviceID ,containerID,postion,count,classID,itemType")
		ret = Dmc_HuaweiJse_Read(func, temp, value, len);
		i += 5;
	}
	json_object_put(ret_json);


	func = "ContItemNumber_Get";
	classID  = "1";
	itemType = "2";
	sprintf(temp, "%s,%s,%s,%s", udn, objectID, classID, itemType);
	value[0]=0;
	ret = Dmc_HuaweiJse_Read(func, temp, value, len);
	printf("----image: %s\n\r", value);

	ret_json = dlna_json_tokener_parse(value);
	ret_num = json_object_object_get(ret_json, "items_num");
	num = json_object_get_int(ret_num);
	for(i = 0; i < num; )
	{
		func = "ContItemList_Get";
		sprintf(temp, "%s,%s,%d,%d,%s,%s", udn, objectID, i, i+5, classID, itemType);
		value[0]=0;
		//sJson=iPanel.ioctlRead("ContItemList_Get: deviceID ,containerID,postion,count,classID,itemType")
		ret = Dmc_HuaweiJse_Read(func, temp, value, len);
		i += 5;
	}
	json_object_put(ret_json);


/* test audio int root folder */
	func = "ContItemNumber_Get";
	objectID = "2";
	classID  = "1";
	sprintf(temp, "%s,%s,%s", udn, objectID, classID);
	value[0]=0;
	ret = Dmc_HuaweiJse_Read(func, temp, value, len);
	printf("----audio: %s\n\r", value);


	sprintf(temp, "%s,%s,%s,", udn, objectID, classID);
	value[0]=0;
	ret = Dmc_HuaweiJse_Read(func, temp, value, len);
	printf("----audio: %s\n\r", value);


	sprintf(temp, "%s,%s,%s, ", udn, objectID, classID);
	value[0]=0;
	ret = Dmc_HuaweiJse_Read(func, temp, value, len);
	printf("----audio: %s\n\r", value);
	ret_json = dlna_json_tokener_parse(value);

	itemType = "1";
	ret_num = json_object_object_get(ret_json, "containers_num");
	num = json_object_get_int(ret_num);
	//sJson=iPanel.ioctlRead("ContItemList_Get: deviceID ,containerID,postion,count,classID,itemType")
	for(i = 0; i < num; )
	{
		func = "ContItemList_Get";
		sprintf(temp, "%s,%s,%d,%d,%s,%s", udn, objectID, i, i+5, classID, itemType);
		value[0]=0;
		ret = Dmc_HuaweiJse_Read(func, temp, value, len);
		i += 5;
	}

	itemType = "2";
	ret_num = json_object_object_get(ret_json, "items_num");
	num = json_object_get_int(ret_num);
	//sJson=iPanel.ioctlRead("ContItemList_Get: deviceID ,containerID,postion,count,classID,itemType")
	for(i = 0; i < num; )
	{
		func = "ContItemList_Get";
		sprintf(temp, "%s,%s,%d,%d,%s,%s", udn, objectID, i, i+5, classID, itemType);
		value[0]=0;
		ret = Dmc_HuaweiJse_Read(func, temp, value, len);
		i += 5;
	}
	json_object_put(ret_json);


/*test many images > 900*/
	func = "ContItemNumber_Get";
	objectID = "133";
	classID  = "1";
	sprintf(temp, "%s,%s,%s,   ", udn, objectID, classID);
	value[0]=0;
	ret = Dmc_HuaweiJse_Read(func, temp, value, len);
	printf("---->1000: %s\n\r", value);
	ret_json = dlna_json_tokener_parse(value);

	itemType = "1";
	ret_num = json_object_object_get(ret_json, "containers_num");
	num = json_object_get_int(ret_num);
	//sJson=iPanel.ioctlRead("ContItemList_Get: deviceID ,containerID,postion,count,classID,itemType")
	for(i = 0; i < num; )
	{
		func = "ContItemList_Get";
		sprintf(temp, "%s,%s,%d,%d,%s,%s", udn, objectID, i, i+5, classID, itemType);
		value[0]=0;
		ret = Dmc_HuaweiJse_Read(func, temp, value, len);
		i += 5;
	}

	itemType = "2";
	ret_num = json_object_object_get(ret_json, "items_num");
	num = json_object_get_int(ret_num);
	//sJson=iPanel.ioctlRead("ContItemList_Get: deviceID ,containerID,postion,count,classID,itemType")
	//for(i = 0; i < num; )
	{
		func = "ContItemList_Get";
		sprintf(temp, "%s,%s,%d,%d,%s,%s", udn, objectID, i, i+5, classID, itemType);
		value[0]=0;
		ret = Dmc_HuaweiJse_Read(func, temp, value, len);
		i += 5;
	}
	json_object_put(ret_json);



//1. create handle
    //sValue=iPanel.ioctlRead("PlayerInstance_Creat"£¬"Instance_Type")
    value[0]=0; func = "PlayerInstance_Creat";
    ret = Dmp_HuaweiJse_Read(func, para, value, len);

//2. play
//	iPanel.ioctlWrite("Local_Play","{"itemID":itemID,"entryID":entryID}")
    value[0]=0; func = "Local_Play";
    strcpy(value, "{\"itemID\":\"18\",\"entryID\":18}");
    ret = Dmp_HuaweiJse_Write(func, para, value, len);
	sleep(10);
//3. stop
//	iPanel.ioctlWrite("Local_Stop"£¬"XXXX")
	value[0]=0; func = "Local_Stop";
	ret = Dmp_HuaweiJse_Write(func, para, value, len);
#endif
#endif
#if 0
	int n = 10;
	while(n)
	{
		mid_dlna_PauseDms();
		sleep(2);
		mid_dlna_ResumeDms();
		sleep(1);
		mid_dlna_restart();
		sleep(1);
	}
#endif
	return 0;
}

#endif
#endif

