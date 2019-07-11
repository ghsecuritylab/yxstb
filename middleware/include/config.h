#ifndef __CONFIG_H__20070312__
#define __CONFIG_H__20070312__

/********************
*modify list
*zp 2011.1.21
*
*********************/
#ifdef Gansu
#define PPPOE_PADI_REPETITIONS 4 // CTC requirements three times padi failure suggests a connection failure.
#else
#define PPPOE_PADI_REPETITIONS 3 // CTC requirements three times padi failure suggests a connection failure.
#endif
//enum{	//C53 please include mid_network.h
//
//	NET_MSG_PPPOE_PASSWORD_ERROR = 1,
//	NET_MSG_PPPOE_CONNECT_OK,
//	NET_MSG_PPPOE_CONNECT_ERROR,
//	NET_MSG_DHCP_CONNECT_OK,
//	NET_MSG_DHCP_CONNECT_ERROR,
//};

enum{ //C27
	CA_FLAG_NOT_CA_ACCOUNT = 0,
	CA_FLAG_SECURE_MEDIA,
	CA_FLAG_IRDETO_HARD,
	CA_FLAG_IRDETO_SOFT,
	CA_FLAG_VERIMATRIX
};

#define SITCOM_MAX_LEN 4096

enum{
	/*lh  2010-4-23  定义local time shift 消息,与柳建华沟通后确定暂时只处理这两个消息*/
	LOCAL_TS_START = 0x7000, //C27
	LOCAL_TS_DISK_FULL = 0x7001, //C27
	LOCAL_TS_DATA_DAMAGE = 0x7002, //C27
	LOCAL_TS_PVR_CONFLICT = 0x7003, //C27
	LOCAL_TS_DISK_ERROR = 0x7004,
	LOCAL_TS_MSG_ERROR = 0x7005,
	LOCAL_TS_DISK_DETACHED = 0x7006,
/*
	// pvr的消息
	//PVR_FILE_CREATED=	0x8000,	
	PVR_MSG_ERROR=	0x8001,
	PVR_MSG_DISK_NOT_FOUNDED=0x8002,
	//PVR_MSG_DISK_NOT_SUPPORTED=0x8003,
	PVR_MSG_DISK_FULL=	0x8004,
	//PVR_MSG_TOO_MANY=	0x8005,		
	PVR_MSG_SUCCESS_BEGIN=0x8006,
	PVR_MSG_SUCCESS_END=0x8007,
	PVR_MSG_CLOSE=	0x8008,
	PVR_MSG_RECV_END=	0x8009,
	//PVR_MSG_RECV_TIMEOUT=0x800a,	
	//PVR_MSG_RECV_RESUME=0x800b,	
	//PVR_MSG_NONE=	0x800c,	
	//PVR_MSG_MAX=		0x800d,
	PVR_SCHEDULE_NOTIFY=0x800e,
	//PVR_MSG_TOO_SHORT=	0x800f,	//C27
	PVR_SCHEDULE_REAL_START=0x8010,
	PVR_STOP_IN_RECORDING=0x8011,	//C27 //wangjian 停止正在录制中的PVR
*/
/*
	//dlna msg ==// move form 0x997x-> 0x81xx	
	DLNA_MEDIA_PLAY_REQ=0x8101,
	DLNA_DHCP_IP_CHANGE=0x8102,
	DLNA_PLAY_SELECT_OK=0x8103,
	DLNA_PLAY_SELECT_CANCEL=0x8104,
	DLNA_PLAY_IDLE=0x8105,
	DLNA_PLAY_START=0x8106,
	DLNA_PLAY_STOP=0x8107,
	DLNA_PLAY_PAUSE=0x8108,
	DLNA_PLAY_FAST_FORWARD=0x8109,
	DLNA_PLAY_FAST_BACKWARD=0x810a,
*/
	//TB_MONITOR_OPEN_UPGRADE = 0x8800, //打开工具升级页面
    NETWORK_CARD_UP = 0x8FFE,
    NETWORK_CARD_DOWN = 0x8FFF,
	//NETWORK_CONNECT = 0x9000, // MessageValueNetwork.h
	//NETWORK_DISCONNECT = 0x9001,
	//NETWORK_IP_UNCONFLICT = 0x9002,
	//NETWORK_IP_CONFLICT = 0x9003,

	RESTART_AUTH = 0x9004, //C53
	SECOND_EDS = 0x9005, //C53	
	NETWORK_CONNECT_OK = 0x9006,
	NETWORK_DHCP6_EXIT = 0x9007,
	NETWORK_CONNECT_ERROR = 0x9008, //for show message in boot
	//0x92xx reserved for upgrade
	//SEEK_TIME_OK = 0x9400, //C53	
	//SEEK_TIME_ERROR = 0x9401, //C53
	NTP_SYNC_OK = 0x9500,
	NTP_SYNC_ERROR = 0x9501,
	NTP_DNS_SERVER_NOTFOUND = 0x9502, //C27
	NTP_DNS_RESOLVE_ERROR =	0x9503,	//C27
	OPEN_CONNECT_URL = 0x950d, //C53
	NET_CONNECT = 0x950E, //C53
	DHCP_CONNECT_OK = 0x9510, //for web use, please don't modify
	DHCP_CONNECT_ERROR = 0x9511, //for web use, please don't modify

#ifdef ENABLE_DBLVLAN 
    DHCP_PRIVATE_NET_CONNECT_FAIL,
	DHCP_PUBLIC_NET_CONNECT_FAIL,
	DHCP_WRONG_INTERFACE,
	DHCP_PRIVATE_NET_CONNECT_OK,
	DHCP_PRIVATE_NET_CONNECT_ERROR,
	DHCP_PUBLIC_NET_CONNECT_OK,
	DHCP_PUBLIC_NET_CONNECT_ERROR,
	DHCP_PRIVATE_NET_CONNECT_ONLY,
#endif

	//UPGRADE_NEED = 0x9520,	
	//UPGRADE_UNNEED = 0x9521,
	//TIMEOUT_AUDIO = 0x9530,
	//TIMEOUT_BGMUSIC = 0x9531, //没有发送源，暂时注释
	//TIMEOUT_SEEK = 0x9532,	
	//TIMEOUT_STATUS = 0x9533, //播放状态显示超时
	//TIMEOUT_PROCESS = 0x9534,	
	//SWITCH_TIMEOUT = 0x9536, //C53
	//PPV_SIGNAL = 0x9538, //C53 0x9532 没有发送源，暂时注释
	//PPV_SIGNAL_TIMEOUT = 0x9539, //C53 0x9533	add comment for useless
	PPV_SIGNAL_END = 0x9539,
	//PPV_PAUSE_TIMEOUT = 0x953a,	//C53 0x9534
	//PPV_PLTV_TIMEOUT =	0x953b,	//C53 0x9535
	//PLAY_PAUSE_TIMEOUT = 0x953c,	//C53 0x9539 //暂时无人调用
	TR069_REQUEST_REBOOT = 0x9540,
	//REQUEST_RESET=	0x9541,	//C53

	SM_INIT_OK = 0x9551, //C27	for web use, please don't modify
	SM_INIT_ERROR = 0x9552, //C27	for web use, please don't modify

	PPPOE_CONNECT = 0x9580, //for web use, please don't modify
	PPPOE_CONNECT_ERROR = 0x9581, //for web use, please don't modify
	PPPOE_PASSWORD_ERROR = 0x9583, //for web use, please don't modify
	PPPOE_SERVER_LOST = 0x102006, //C27 //for web use, please don't modify
	STATIC_IP_CONNECT_OK = 0x9584, //C53	?

	ITMS_OK = 0x9585, //for web use, please don't modify
	ITMS_ERROR = 0x9586, //for web use, please don't modify

	ITMS_START = 0x9587,
	ITMS_STOP = 0x9588,
	ITMS_OPEN_SERIAL_NUM_BOX = 0x9589,
	//VOD_PLAY_OVER_ERROR = 0x9601,	//C27 add comment for useless
	//VOD_NET_ERR = 0x9602,	//C27 add comment for useless
	//STREAM_STATE_CHANGE = 0x9702, //C27	
	//BEGIN_OF_STREAM = 0x9704, //C27	
	//SEEKOUT_OF_STREAM = 0x9705, //C27	
	//DISPLAY_OF_STREAM = 0x9706, //C27	
	//FORCE_CLOSE_STREAM = 0x9707, //C27	
	//ERROR_OF_STREAM = 0x970a, //C27 上层没有处理，暂时注释
	//MUSIC_OF_STREAM = 0x970c, //上层没有处理，暂时注释
	//HEART_BIT_OK = 0x9810, //上层没有处理，暂时注释
	//HEART_BIT_ERROR = 0x9811, //上层没有处理，暂时注释
	HEART_BIT_RUN = 0x9812,

	CHANNEL_LIST_OK = 0x9820,
	CHANNEL_LIST_ERROR = 0x9821,
	CHANNEL_PPV_OK = 0x9822,
	CHANNEL_PPV_ERROR = 0x9823,
	//PROG_INFO_OK = 0x9824, //C53 0x9830	add comment for useless
	//PROG_INFO_ERROR = 0x9825, //C53 0x9831	add comment for useless
	//PLAY_BILL_OK = 0x9826, //C53 0x9840	add comment for useless
	//PLAY_BILL_ERROR = 0x9827, //C53 0x9841	add comment for useless
	//VOD_OPEN_OK = 0x9828, //C53 0x9850	add comment for useless
	//VOD_OPEN_ERROR = 0x9829, //C53 0x9851	add comment for useless
	REMINDER_LIST_OK = 0x9830, //C27

	//IPTV_SAVE_CHANNEL= 0x9834, //C27
	//IPTV_TIMEOUT_MUSIC = 0x9836, //C27 add comment for useless
	//IPTV_TIMEOUT_BASEINFO = 0x9837, //C27 add comment for useless //频道号显示超时
	//IPTV_TIMEOUT_PIP = 0x9838, //C27	add comment for useless
	//IPTV_TIMEOUT_SWITCH = 0x9839, //C27	add comment for useless
	//IPTV_TIMEOUT_NUMBER = 0x983a, //C27	add comment for useless
	//IPTV_TIMEOUT_SWITCH_PIP = 0x983b, //C27 add comment for useless
	TELETEXT_TIMEOUT_NUMBER = 0x983c,	//C27
	FLASH_FLV_END = 0x983d,

	//for chan logo
	//CHANNEL_LOGO_OK = 0x9845,	//C27
	//CHANNEL_LOGO_ERROR = 0x9846, //C27
	//DOWNLOAD
	DOWNLOAD_CHECK = 0x9851, //C53 0x9950 没有发送源，暂时注释
	//DOWNLOAD_START = 0x9852,	//C53 0x9951	

	//format
	HD_FORMAT_XFS_OK = 0x9861, //C27 for web use, please don't modify
	HD_FORMAT_XFS_FAILD = 0x9862, //C27 for web use, please don't modify
	//IPTV_OPEN_OK = 0x9860, //C53 add comment for useless
	//IPTV_OPEN_ERROR = 0x9861, //C53 conflict, add comment for useless
	//OPEN_TRANSPARENT_PAGE = 0x9862,	//C53 conflict, C27 defined in ipanel_event.h

	/*add by nizheng 080319 for reminder*/
	//REMINDER_RUN = 0x9866,
	//TVMS_HEART_RUN = 0x9867, //C27
	//TVMS_VOD_RUN = 0x9868, //C27
	//CLIST_SCAN_RUN = 0x9869,	
	//VLIST_SCAN_RUN = 0x9870,

	/*本地显示trick 图标
	SHOW_RATE_ICON = 0x9880,
	SHOW_LIVE_ICON = 0x9881,
	SHOW_END_ICON = 0x9882,
	SHOW_BEGIN_ICON = 0x9883,
	SHOW_HIDE_TRICKICON = 0x9884,
	HIDE_AUDIO_BAR = 0x9885,
	SHOW_MUTE_BAR = 0x9886,
	SHOW_MUSIC_PLANE = 0x9889,
	HIDE_MUSIC_PLANE = 0x988A,
//	SHOW_LOAD_PLANE = 0x988B,	//C27
//	HIDE_LOAD_PLANE = 0x988C,	//C27
	SHOW_WATCH_PLANE = 0x988E,
    */
	KEYTABLE_UPDATE = 0x9887,
    //JOINCHANNEL_MSG =	0x9888,
	//MEDIA_PLAY_ERROR = 0x9890,	//0x988B->0x9890 //C53 conflict	
	//MEDIA_PLAY_RESUME = 0x9891,	//0x988C->0x9891 //C53 conflict
	//MEDIA_CANNOT_PLAY = 0x9892,	//0x988D->0x9892 //C53 conflict	
	//u_config message
	USB_CONFIG_OK = 0x98AA,
	USB_UPGRADE_OK = 0X98AB, //LH  porting  from B260_HCC

	HEART_USER_INVALID = 0x9901, //0x9813 modified for B200,	//for epg use, please don't modify
	//PREVIEW_COUNT_END = 0x9902,	//预览次数已到//上层没有处理，暂时注释
	//PREVIEW_TIME_END = 0x9903,	//预览时间已到//上层没有处理，暂时注释
	INVAILD_CHANNEL=0x9904, //广东局点需要，所以打开
	//STREAM_PAUSE_TIMEOUT = 0x9911, //C27 add comment for useless
	//STREAM_SEEK_OUT_RANG = 0x9912, //C27 add comment for useless
	//STREAM_USER_CLOSE = 0x9913, //C27 add comment for useless
	//STREAM_DATA_TIMEOUT = 0x9914, //C27 add comment for useless
	//STREAM_DATA_RESUME = 0x9915, //C27 add comment for useless
	//STREAM_USER_SUSPEND = 0x9916, //C27 add comment for useless
	//STREAM_PLAY_ERROR = 0x9917, //C27 add comment for useless

	//IPTV_CHANNEL_PIP_VIEW = 0x9931, //0x9831->0x9931,	//C27 上层没有处理，暂时注释
	//IPTV_CHANNEL_SWITCHING = 0x9932, //C27 add comment for useless
	//IPTV_CHANNEL_SWITCHED = 0x9933, //C27 add comment for useless
	//IPTV_CHANNEL_LOCKED = 0x9934, //C27 add comment for useless
	//IPTV_CHANNEL_UNAUTHOR = 0x9935, //C27	add comment for useless
	//pvr
	//END_OF_STREAM = 0x9952,	
	//PVR_TASK_CONFLICT = 0x9961,

	//0x996x - 0x998x reserved
	//PVR_CHANNELLIST_END = 0x9971,	//C53 0x9971 //频道列表下发完成，上层没有处理，暂时注释

	//DOWNLOAD_GAME_PLAY_GAME = 0x9995,	//C53
	//DOWNLOAD_GAME_RETURN_EPG_LOCALPLAYE = 0x9996, //C53
	//DOWNLOAD_GAME_RETURN_EPG_LOCALGAME = 0x9997, //C53

	//BHARAL_IR_KEY_HEARTBEAT_VALUE=0xff41,	// C53
	/***************************************************************************
	  ——————————————    external    ————————————
	 ****************************************************************************/
	//PVR_SPACE_NOTENOUGH = 9965,//C53 0x9965	
	//PVR_SPACE_FULL = 9966,	//C53 0x9966
	//PVR_DISK_ERR = 9967,	//C53 0x9967
	//PVR_SCHEDULE_PREDICT = 9968,//C53	0x9968	
	//PVR_SCHEDULE_START = 9969,//C53	0x9969
	//PVR_SCHEDULE_END = 9970,//C53	0x9970

	//wangjian maybe 根据Losu03定义的事件类型，但与已有的冲突!
	//PVR_SCHEDULE_PARTIAL_END = 9971,//C27	
	//PVR_SCHEDULE_FAILED = 9972,	//C27
	//wangjian 以下4个宏的值待定
	//PVR_SPACE_TOOSHORT = 9976, //C27
	//PVR_LOCALTS_START = 9977,	//C27
	//PVR_LOCALTS_FULL = 9978, //C27	
	//PVR_LOCALTS_BW_LIMITED = 9979, //C27	

	/* //download
	//DOWNLOAD_END = 9980, //C53 0x9980
	DOWNLOAD_BUF_READY = 9981, //C53 0x9981
	DOWNLOAD_MEDIA_STOP = 9982, //C53 0x9982
	DOWNLOAD_BUF_EMPTY = 9983, //C53 0x9983
	DOWNLOAD_MEDIA_404 = 9987, //C53 0x9987
	DOWNLOAD_SERVER_BUSSY = 9988, //C53 0x9988
	DOWNLOAD_SERVER_ERR = 9989, //C53 0x9989
	*/

	//dvb-s add. by lidong  2011.04.01
	DVBS_SIGNAL_WEEK = 0x9996, //for epg use, please don't modify
	DVBS_SIGNAL_POWER = 0x9997, //for epg use, please don't modify
	//DVBS_TEMPLATE_SWITCH = 0x9998, //dvbs单独模板更新功能
	//DVBS_FIRST_KISS_OVER = 0x9999, //自动搜索功能
	DVBS_CHDB_START_OK =0x999a, //for epg use, please don't modify
	DVBS_CHDB_START_ERR = 0x999b, //for epg use, please don't modify
	DVBS_CHDB_TIMEOUT_ERR = 0x999c, //for epg use, please don't modify
	DVBS_SW_TIMEOUT_ERR = 0x999d, //for epg use, please don't modify
    CHECK_VERSION_VALIDITY = 0x999E,
    LITTLE_SYSTEM_RUN = 0x999F, 

    ERRORCODE_BASE,
    LITTLE_SYSTEM_UDISK_UPGRADE_FILAID = ERRORCODE_BASE + 10,
    LITTLE_SYSTEM_SERVER_UPGRADE_FILAID = ERRORCODE_BASE + 11,

#ifdef ENABLE_DBLVLAN 
    ERRORCODE_DHCP_CONNECT_PRIVATE_FAIL = ERRORCODE_BASE+100,
    ERRORCODE_DHCP_CONNECT_PUBLIC_FAIL = ERRORCODE_BASE+101,
    ERRORCODE_DHCP_CONNECT_WRONG_INTERFACE = ERRORCODE_BASE+102,
#endif
    TR069_NET_SPEED_TEST = ERRORCODE_BASE + 103,

    /*epg error show by epg*/
    ERRORCODE_OVER = ERRORCODE_BASE + 200,
    HM_CLOSEDIALOG,
};

#endif /* __CONFIG_H__ */

