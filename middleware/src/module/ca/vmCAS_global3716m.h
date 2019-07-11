#ifndef __vmip_global_h__
#define __vmip_global_h__


/************************************************************************************************/

/* invalid pid value */
#ifndef 	VMCAS_PSI_BAD_PID_VALUE
#define 	VMCAS_PSI_BAD_PID_VALUE							0x1FFF
#endif

#ifndef		VMCAS_PSI_INVALID_TABLE_ID
#define 	VMCAS_PSI_INVALID_TABLE_ID					0xFF00
#endif

#ifndef 	VMCAS_PSI_MAX_PID_VALUE
#define 	VMCAS_PSI_MAX_PID_VALUE							0x1FFF
#endif

/* invalid program number value */
#ifndef		VMCAS_PSI_INVALID_PROGRAM_NUMBER
#define		VMCAS_PSI_INVALID_PROGRAM_NUMBER		0x1FFFF
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
}VMCAS_CONFIG_TYPE;

/* show verimatrix porting version number */
void vmCAS_app_api_version( void);

/* check init error and report error code */
int vmCAS_api_initErr_set_callback( int (*vmCAS_api_init_error_callback)( int wAcsId,int wChannel,int wError ));

/* init verimatrix */
int vmCAS_app_api_init( int wAcsNumber );

/* set configration file */
int vmCAS_app_api_set_configfile( VMCAS_CONFIG_TYPE enType, const char * pConfigValue);

/* get configration file */
int vmCAS_app_api_get_configfile( VMCAS_CONFIG_TYPE enType, char * pConfigValue);

/* decrypt stream */
int vmCAS_app_api_decrypt_stream( int dwAcsId, char *pTsBuf, int wTsBufSize, int wEcmPid, unsigned int wPhyAddr );

int vmCAS_app_api_MosicDecryptSoftDecryption( int dwAcsId, char* pStream, int wStreamLength );

/* get cw flag */
int vmCAS_app_api_check_KeyIsReady( int dwAcsId, int *pwkeyIsReady );

/* reset cw flag */
int vmCAS_app_api_resetCwIsReady( int dwAcsId );

/* upgrade channel key */
int vmCAS_app_api_upgrade_channelkey( int dwAcsId );

/* reset stream */
int vmCAS_app_api_reset_stream( int dwAcsId );

/* reset masic stream */
int vmCAS_app_api_reset_masicstream( int dwAcsId );

#endif
