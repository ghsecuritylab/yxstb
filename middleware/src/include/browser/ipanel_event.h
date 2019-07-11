/*********************************************************************
    Copyright (c) 2005 Embedded Internet Solutions, Inc
    All rights reserved. You are not allowed to copy or distribute
    the code without permission.
    There are the Porting Events needed by iPanel MiddleWare.

    Note: the "int" in the file is 32bits

    $ver0.0.0.1 $author Zouxianyun 2005/04/28
*********************************************************************/

#ifndef _IPANEL_MIDDLEWARE_PORTING_EVENT_H_
#define _IPANEL_MIDDLEWARE_PORTING_EVENT_H_

	/*	messages same in Event[0] area.	*/
typedef enum{
	IPANEL_EVENT_TYPE_TIMER    =0 ,
	IPANEL_EVENT_TYPE_SYSTEM = 1,
	IPANEL_EVENT_TYPE_KEYDOWN =2,
	IPANEL_EVENT_TYPE_KEYUP =3,
	IPANEL_EVENT_TYPE_NETWORK =4,
	IPANEL_EVENT_TYPE_CHAR = 5,
	IPANEL_EVENT_TYPE_MOUSE =6,
	IPANEL_EVENT_TYPE_IRKEY =7,
	IPANEL_EVENT_TYPE_IPTV = 14,
	IPANEL_EVENT_TYPE_DVB = 0x100,
}IPANEL_EVENT_MSG;

  /** wpara messages same in Event[1] area, but effect only message = EIS_EVENT_TYPE_TIMER.*/
enum{
	EIS_TIMER_TIMER = 0x0
};

  /** wpara messages same in Event[1] area, but effect only message = EIS_EVENT_TYPE_NETWORK.*/
enum {
	SOCKET_EVENT_TYPE_CONNECT = 0x1,
	SOCKET_EVENT_TYPE_CLOSE,
	SOCKET_EVENT_TYPE_READ
};

  /** wpara messages same in Event[1] area (ASCII), but effect only message = EIS_EVENT_TYPE_CHAR.*/
  /** wpara messages same in Event[1] area, but effect only message = EIS_EVENT_TYPE_MOUSE.*/
typedef enum{
	IPANEL_MOUSE_MOUSEMOVE,
	IPANEL_MOUSE_LBUTTONDOWN,
	IPANEL_MOUSE_LBUTTONUP,
	IPANEL_MOUSE_RBUTTONDOWN,
	IPANEL_MOUSE_RBUTTONUP,
	IPANEL_MOUSE_UNDEFINED
}IPANEL_MOUSE_STATUS;

#define MAKE_MOUSE_POS(t, x, y)     (((((long)(x)) & 0xffff) << 16) | ((((long)(y)) & 0xffff)))
#define GET_MOUSE_XPOS(e)           (((e) >> 16) & 0xffff)
#define GET_MOUSE_YPOS(e)           ((e) & 0xffff)


  /** wpara messages same in Event[1] area, but effect only message = EIS_EVENT_TYPE_IRKEY.*/

typedef enum{
	EIS_IRKEY_NULL	= 0x0100,/* bigger than largest ASCII*/
	EIS_IRKEY_STAR = 0xCB,
    EIS_IRKEY_POWER = 0x0101,
    EIS_IRKEY_FPANELPOWER=0x0102,
    KEY_BACKSPACE = 0x008,
    KEY_POWER_CONTINUOUS = 0X0085,	/*If press powerKey more than 300ms, send this key to EPG to enter deep standby.*/
    KEY_OPTIONS = 0x0086,
    KEY_VOD = 0X0018,
    KEY_GO_END = 0x010A,
    KEY_GO_BEGIN = 0x010B,
    EIS_KEY_DVB_TXT = 0x010D,	/* open/close the teletext information */
    /* generic remote controller keys */
    EIS_IRKEY_NUM0  = 0x0110,
    EIS_IRKEY_NUM1,
    EIS_IRKEY_NUM2,
    EIS_IRKEY_NUM3,
    EIS_IRKEY_NUM4,
    EIS_IRKEY_NUM5,		/* 0x115 */
    EIS_IRKEY_NUM6,
    EIS_IRKEY_NUM7,
    EIS_IRKEY_NUM8,
    EIS_IRKEY_NUM9,
    EIS_IRKEY_UP,		/* 0x11A */
    EIS_IRKEY_DOWN,
    EIS_IRKEY_LEFT,
    EIS_IRKEY_RIGHT,
    EIS_IRKEY_SELECT,	/* 0x11E */
	/* generic editting/controling keys */
	EIS_IRKEY_INSERT    = 0x130,
	EIS_IRKEY_DELETE,
	EIS_IRKEY_HOME,
	EIS_IRKEY_END,
	EIS_IRKEY_ESC,
	EIS_IRKEY_CAPS,     /* 0x135 */
	EIS_IRKEY_REFRESH = 0x152,
	EIS_IRKEY_EXIT,
	EIS_IRKEY_BACK,     /*backspace*/
	EIS_IRKEY_CANCEL,
	/* generic navigating keys */
	EIS_IRKEY_SCROLL_UP     = 0x170,
	EIS_IRKEY_SCROLL_DOWN,
	EIS_IRKEY_SCROLL_LEFT,
	EIS_IRKEY_SCROLL_RIGHT,
	EIS_IRKEY_PAGE_UP,
	EIS_IRKEY_PAGE_DOWN,    /* 0x175 */
	EIS_IRKEY_HISTORY_FORWARD,
	EIS_IRKEY_HISTORY_BACKWARD,
	EIS_IRKEY_SHOW_URL,
	/* function remote controller keys */
	EIS_IRKEY_VIRTUAL_KEY_START = 0x1A3, //added by tedd.

	EIS_IRKEY_VIRTUAL_KEY_END = 0x1FF,

	EIS_IRKEY_MENU					= 0x200,
	EIS_IRKEY_BTV,									//EIS_IRKEY_HOMEPAGE
	EIS_IRKEY_EPG,
	EIS_IRKEY_HELP,
	EIS_IRKEY_MOSAIC,
	EIS_IRKEY_VOD,
	EIS_IRKEY_TVOD,		//EIS_IRKEY_NVOD
	EIS_IRKEY_SETTING,
	EIS_IRKEY_STOCK,
	/* special remote controller keys */
	EIS_IRKEY_SOFT_KEYBOARD = 0x230,
	EIS_IRKEY_IME,
	EIS_IRKEY_DATA_BROADCAST,
	EIS_IRKEY_VIDEO,            /*视频键*/
	EIS_IRKEY_AUDIO,            /*音频键*/
	EIS_IRKEY_LANGUAGE,     /* 0x235 */
	EIS_IRKEY_SUBTITLE,
	EIS_IRKEY_INFO,
	EIS_IRKEY_RECOMMEND,        /*推荐键*/
	EIS_IRKEY_FORETELL,         /*预告键*/
	EIS_IRKEY_FAVORITE,         /*收藏键*/
	/* 切换键（目前仅用于刷卡遥控器项目作为银联键进行临时演示，如需用于其他用途需要进行讨论）*/
	EIS_IRKEY_STATUS = 0x23C,
	EIS_IRKEY_PLAYLIST,
	EIS_IRKEY_PROGRAM_TYPE,
	/* user controling remote controller keys */
	EIS_IRKEY_LAST_CHANNEL  = 0x250,
	EIS_IRKEY_CHANNEL_UP,
	EIS_IRKEY_CHANNEL_DOWN,
	EIS_IRKEY_VOLUME_UP,
	EIS_IRKEY_VOLUME_DOWN,
	EIS_IRKEY_VOLUME_MUTE,
	EIS_IRKEY_AUDIO_MODE,
	/* virtual function keys */
	EIS_IRKEY_VK_F1         = 0x300,
	EIS_IRKEY_VK_F2,
	EIS_IRKEY_VK_F3,
	EIS_IRKEY_VK_F4,
	EIS_IRKEY_VK_F5,
	EIS_IRKEY_VK_F6,        /* 0x305 */
	EIS_IRKEY_VK_F7,
	EIS_IRKEY_VK_F8,
	EIS_IRKEY_VK_F9,
	EIS_IRKEY_VK_F10,
	EIS_IRKEY_POUND = EIS_IRKEY_VK_F10, //added by teddy at 2011.02.17 Thur 15:31:30
	EIS_IRKEY_VK_F11,       /* 0x30A */
	EIS_IRKEY_VK_F12,
	/* special function keys class A */
	EIS_IRKEY_FUNCTION_A    = 0x320,
	EIS_IRKEY_FUNCTION_B,
	EIS_IRKEY_FUNCTION_C,
	EIS_IRKEY_FUNCTION_D,
	EIS_IRKEY_FUNCTION_E,
	EIS_IRKEY_FUNCTION_F,
	/* special function keys class B */
	EIS_IRKEY_RED           = 0x340,
	EIS_IRKEY_GREEN,
	EIS_IRKEY_YELLOW,
	EIS_IRKEY_BLUE,
	EIS_IRKEY_ASTERISK = 0X350,
	EIS_IRKEY_NUMBERSIGN,
	/* VOD/DVD controling keys */
	EIS_IRKEY_PLAY = 0x400,
	EIS_IRKEY_STOP,
	EIS_IRKEY_PAUSE,
	EIS_IRKEY_RECORD,
	EIS_IRKEY_FASTFORWARD,
	EIS_IRKEY_REWIND,
	EIS_IRKEY_STEPFORWARD,
	EIS_IRKEY_STEPBACKWARD,
	EIS_IRKEY_DVD_AB,
	EIS_IRKEY_DVD_MENU,
	EIS_IRKEY_DVD_TITILE,
	EIS_IRKEY_DVD_ANGLE,
	EIS_IRKEY_DVD_ZOOM,
	EIS_IRKEY_DVD_SLOW,
	EIS_IRKEY_COMM,	//EIS_IRKEY_TV_SYSTEM,
	EIS_IRKEY_DVD_EJECT,
	EIS_IRKEY_FAVOR = 0x41a,
	EIS_IRKEY_INTERX = 0x420,
	EIS_IRKEY_AUDCHNL = 0x424,//上海高清播放器长按声道键需要发送0x424+0xff00

	EIS_IRKEY_SEARCH			= 0x451,
	EIS_IRKEY_NPVR				=	0x453,
	KEY_TV_MENU  =0x454,
	KEY_VOD_MENU = 0x455,
	KEY_TVOD_MENU = 0x456,
	EIS_IRKEY_PVR					= 0x457,
	EIS_IRKEY_SWITCH 			= 0x458,
	EIS_IRKEY_NVOD,
	EIS_IRKEY_OPTION= 0x460,
	KEY_REC_ACTIVE  = 0x45C,
	EIS_KEY_EXIT =0x45D,
	EIS_KEY_PROGRAM_GUIDE = 0x45E,	//program guid
	EIS_KEY_RECORDINGS = 0x045F,	//boot up recording



/*	//--added by ZXM  */
	EIS_IRKEY_CLEAR				= 0x500,
	EIS_IRKEY_GOTO				= 0x501,
	/* EIS_IRKEY_SUBTITLE			= 0x502, */
	EIS_IRKEY_TRACK				= 0x503,
/*	//	EIS_IRKEY_AUDIO				= 0x424, */
	EIS_IRKEY_TRANS_ENTER		= 0x505,
	EIS_IRKEY_TRANS_BACK		= 0x506,
	EIS_IRKEY_TRANS_EXIT		= 0x507,
	/*一个URL同时拥有FAST和URL两个键值目录是在VOD模式下FAST能透过mid_key_translate传递到网页*/

/*	//	EIS_IRKEY_NVOD				= 0x50a, */

/*	//	EIS_IRKEY_VOD				= 0x50c, */
/*	//EIS_IRKEY_INFO				= 0x50d, */
	EIS_IRKEY_CAPTION = 1292,	/*0x50c*/
	EIS_IRKEY_DESKTOP = 1293, //0x50D
/*	//	EIS_IRKEY_FAVOR				= 0x41a,  //0x50f */
	EIS_IRKEY_INTER				= 0x510,
/*	//EIS_IRKEY_HELP				= 0x511,  //0x511 */
	EIS_IRKEY_LIVE				= 0x512,
	EIS_IRKEY_FAST_PPV			= 0x513,
/*	//	EIS_IRKEY_MOSAIC			= 0x514, */
	EIS_FASTPAGE_NOTE			= 0x51e,
	EIS_FASTPAGE_OPEN			= 0x51f,
	EIS_IRKEY_UPGRADE_BY_TOOL		= 0x520,
	EIS_IRKEY_NO_NAME,
    EIS_IRKEY_INFO_BLUETOOTH,

	//Added by xzm
	EIS_IRKEY_SPACEBAR	= 		0x800,
	EIS_IRKEY_A			=		0x801,
	EIS_IRKEY_B,
	EIS_IRKEY_C,
	EIS_IRKEY_D,
	EIS_IRKEY_E,
	EIS_IRKEY_F,
	EIS_IRKEY_G,
	EIS_IRKEY_H,
	EIS_IRKEY_I,
	EIS_IRKEY_J,
	EIS_IRKEY_K,
	EIS_IRKEY_L,
	EIS_IRKEY_M,
	EIS_IRKEY_N,
	EIS_IRKEY_O,
	EIS_IRKEY_P,
	EIS_IRKEY_Q,
	EIS_IRKEY_R,
	EIS_IRKEY_S,
	EIS_IRKEY_T,
	EIS_IRKEY_U,
	EIS_IRKEY_V,
	EIS_IRKEY_W,
	EIS_IRKEY_X,
	EIS_IRKEY_Y,
	EIS_IRKEY_Z			= 	0x81A,
	//EIS_IRKEY_STAR		= 	0x81B,
	EIS_IRKEY_GO_END	= 	0x820,
	EIS_IRKEY_GO_BEGINNING,
	EIS_IRKEY_POS,
	EIS_IRKEY_GREY     		= 	0x830,
	EIS_IRKEY_BOOKMARK,
	EIS_IRKEY_CHANNEL_POS,

	EIS_IRKEY_TV_IPTV	= 	0x840,
	EIS_IRKEY_TV_PC,
	EIS_IRKEY_SOURCE,
	EIS_IRKEY_PIP,
	EIS_IRKEY_TV_POWER,
	EIS_IRKEY_TV_SET,
	EIS_IRKEY_TV_CHAN_UP,
	EIS_IRKEY_TV_CHAN_DOWN,
	EIS_IRKEY_TV_VOL_UP,
	EIS_IRKEY_TV_VOL_DOWN	= 0x849,
	EIS_IRKEY_MOVE			=0x84a,

	EIS_IRKEY_MAX				= 4999,
	EIS_IRKEY_URL_MENU			= 5000, /* 0x1388 */
	EIS_IRKEY_URL_END,				/* 0x1389 */
	EIS_IRKEY_PAGE_BTV,				/* 0x138A */
	EIS_IRKEY_PAGE_VOD,				/* 0x138B */
	EIS_IRKEY_PAGE_BOOT,				/* 0x138C */
	EIS_IRKEY_PAGE_CONFIG,			/* 0x138D */
	EIS_IRKEY_PAGE_STANDBY,			/* 0x138E */
	EIS_IRKEY_PAGE_ERROR,				/* 0x138F */
	EIS_IRKEY_PAGE_TIMEOUT,			/* 0x1390 */
	EIS_IRKEY_PAGE_PPV,				/* 0x1391 */
	EIS_IRKEY_NET_UNCONNECT,        //0x1392
    EIS_IRKEY_USB_INSERT,			//0x1393
    EIS_IRKEY_USB_UNINSERT,			//0x1394
	EIS_IRKEY_PAGE_DVBS,				//0x1395
    EIS_IRKEY_PAGE_TRANSPARENT,    //0x1396 //打开透明页面,并改为半播控方式.


    EIS_IRKEY_CHANNEL_LOGO_LOAD,
    EIS_IRKEY_CHANNEL_LOGO_SHOW,
    EIS_IRKEY_CHANNEL_LOGO_SWITCH,

	EIS_IRKEY_SHOWSTANDBY,
	EIS_IRKEY_MODIFY_PPPOEACCOUNT,
	EIS_IRKEY_MODIFY_PPPOEPWD,
	EIS_IRKEY_PAGE_CHECK_PPPOEACCOUNT,
#ifdef INCLUDE_DMR
	EIS_IRKEY_DLNA_PUSH,
	EIS_IRKEY_URG_REG,
#endif

#ifdef WENGUANG_2008
	EIS_IRKEY_STOP_BACK			=	5202,		/* 0x1352 */
#endif
	EIS_IRKEY_CLOSEBROWSER = 0x9990,
	EIS_IRKEY_HDLOCALPLAYER = 0x9998,
	EIS_IRKEY_LOCALPLAYER = 0x9999,
	EIS_IRKEY_UNKNOWN =  0x10000
}IPANEL_IRKEY_VALUE;


  /** wpara messages same in Event[1] area, but effect only message = EIS_EVENT_TYPE_DVB.*/
enum {
	EIS_DVB_DELIVERY_TUNE_SUCCESS = 0x500,
	EIS_DVB_DELIVERY_TUNE_FAILED,
	EIS_DVB_PROGRAM_TUNE_SUCCESS,
	EIS_DVB_PROGRAM_TUNE_FAILED,
	EIS_DVB_SINGAL_QUALITY,
	EIS_DVB_SINGAL_STRENGTH,
	EIS_DVB_SELECT_AV_SUCCESS     = 0x600,
	EIS_DVB_SELECT_AV_FAILED
};

  /** wpara messages same in Event[1] area, but effect only message = EIS_EVENT_TYPE_MESSAGE.
   * This is the struction of p1message. and wpara message is the address of the struction.
   * Note: you must post this message directly, not thought a queque.
   */
typedef int (* p1messageCbf)(int id, int action, unsigned int p1, unsigned int p2);
typedef struct{
    int id;
    int type;
    int priority;
	p1messageCbf cbf;
	unsigned long start_time; /* the seconds from 1970-1-1 00:00:00 */
    unsigned long duration;

  	unsigned int *pos; /*display position, if null is default position, other is a array of x,y,w and h*/
	int modifier;
    int button_lan;
    int msg_len;
    char *msg_text;
    struct Adapt *adapt_field;

	unsigned int running_status;
	void *tmpData;
  } p1message;

enum p1message_Type{
	MESSAGE_TYPE_INSTANT = 0x01,
	MESSAGE_TYPE_ALERT   = 0x02,
	MESSAGE_TYPE_CONFIRM = 0x03,
	MESSAGE_TYPE_PROMPT  = 0x04,
	MESSAGE_TYPE_INPUT   = 0x05,
	MESSAGE_TYPE_SELECT  = 0x06,
	MESSAGE_TYPE_RADIO   = 0x07,
	MESSAGE_TYPE_CHECK   = 0x08,
	MESSAGE_TYPE_STRING  = 0x09,
	MESSAGE_TYPE_MOVESTRING = 0x0a
};

enum p1message_Priority{
	PRIORITY_NONE                        = 0x00,		/*     0 00 00     */
	PRIORITY_TOP_WAIT_CURR               = 0x01,		/*     0 00 01     */
	PRIORITY_TOP_PAUSE_CURR              = 0x05,		/*     0 01 01     */
	PRIORITY_TOP_CANCEL_CURR             = 0x09,		/*     0 10 01     */
	PRIORITY_TOP_WAIT_CURR_RESET_OTHER   = 0x11,		/*     1 00 01     */
	PRIORITY_TOP_PAUSE_CURR_RESET_OTHER  = 0x15,		/*     1 01 01     */
	PRIORITY_TOP_CANCEL_CURR_RESET_OTHER = 0x19,		/*     1 10 01     */
	PRIORITY_NO_CANCEL_CURR_RESET_OTHER  = 0x1a		/*     1 10 10     */
};

enum p1message_Button_language{
	BUTTON_LAN_CHS  = 0x00,
	BUTTON_LAN_CHT  = 0x01,
	BUTTON_LAN_ENG  = 0x02
};

typedef struct Adapt{
	int num_options;
	char **names;
	char **values;
} p1message_Adapt_field_when_type_is_6;

enum p1message_Cbf_parameter_action{
	ACTION_DISCARD = 0x01,
	ACTION_DOMODAL = 0x02
};

enum p1message_Cbf_parameter_p1{
	BUTTON_NONE   = 0x00,
	BUTTON_OK     = 0x01,
	BUTTON_CANCEL = 0x02,
	BUTTON_YES    = 0x04,
	BUTTON_NO     = 0x08
};

  /** wpara messages same in Event[1] area, but effect only message = EIS_EVENT_TYPE_FILTER.*/
enum {
	EIS_FILTER_READ = 0x01,
	EIS_FILTER_FULL	= 0x02
};

  /** wpara messages same in Event[1] area, but effect only message = EIS_EVENT_TYPE_AUDIO.*/
enum {
	EIS_AUDIO_PLAYBUF_COMPLETE = 0x01
};

#endif

