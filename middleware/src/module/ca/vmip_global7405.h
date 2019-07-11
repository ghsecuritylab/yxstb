
#ifndef __vmdvb_global_h__
#define __vmdvb_global_h__




/************************************************************************************************/

/* invalid pid value */
#ifndef 	VMIP_PSI_BAD_PID_VALUE
#define 	VMIP_PSI_BAD_PID_VALUE							0x1FFF
#endif

#ifndef		VMIP_PSI_INVALID_TABLE_ID
#define 	VMIP_PSI_INVALID_TABLE_ID					0xFF00
#endif

#ifndef 	VMIP_PSI_MAX_PID_VALUE
#define 	VMIP_PSI_MAX_PID_VALUE							0x1FFF
#endif

/* invalid program number value */
#ifndef		VMIP_PSI_INVALID_PROGRAM_NUMBER
#define		VMIP_PSI_INVALID_PROGRAM_NUMBER		0x1FFFF
#endif

/************************************************************************************************/

enum
{
	VMIP_PSI_PVR_STATUS_NONE = 0,	/* not pvr playback */
	VMIP_PSI_PVR_STATUS_PLAYBACK,	/* pvr playback */	
	VMIP_PSI_PVR_STATUS_RECORD,		/* record */
	VMIP_PSI_VOD_PLAY,						/* vod play */
};

enum
{
	VMIP_PSI_MAIN_PIC = 0,	/* 0 means main */
	VMIP_PSI_PIP_PIC,		/* 1 means pip */
	VMIP_PSI_PVR_RECORD,	/* 2 means pvr record */	
};

typedef enum
{
	CONFIG_NONE = 0,
	CONFIG_NAME,
	CONFIG_SERV_IP,  
	CONFIG_SERV_PORT,
	CONFIG_VKS,
	CONFIG_UNKNOW = 0XFF
}VMIP_CONFIG_TYPE;



/************************************************************************************************/

/* how many scrambled program can be working at same time */
int vmip_api_init( int wAcsNumber );
int vmip_api_initErr_set_callback( int (*vmip_api_init_error_callback)( int wAcsId,int wChannel,int wError ));
int vmip_api_set_configfile( VMIP_CONFIG_TYPE enType, const char * pConfigValue);
int vmip_api_get_configfile( VMIP_CONFIG_TYPE enType, char * pConfigValue);

int vmip_dscr_api_keyIsReadySet( int wAcsId, int wKeyIsReady );
int vmip_dscr_api_keyIsReadyGet( int wAcsId, int *pwKeyIsReady );
int vmip_psi_pushdata_notify( int dwAcsId, int wEcmPid, char* pbTsPackage, int wLen );

#endif


