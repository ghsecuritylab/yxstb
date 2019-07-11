package com.hybroad.iptv.app;

public class IPTVConstant
{
	public static final int IPTV_MIDDLEWARE_INIT = 0;
	public static final int IPTV_MIDDLEWARE_PAUSE = 1;
	public static final int IPTV_MIDDLEWARE_RESUME = 2;
	public static final int IPTV_MIDDLEWARE_START = 3;
	public static final int IPTV_MIDDLEWARE_RESTART = 4;
	public static final int IPTV_MIDDLEWARE_STOP = 5;
	public static final int IPTV_MIDDLEWARE_RELEASE = 6;
	public static final int IPTV_MIDDLEWARE_SENDKEY = 7;
	public static final int IPTV_MIDDLEWARE_PAGE_CANCELLOAD = 8;
	public static final int IPTV_MIDDLEWARE_AUTH = 9;
	public static final int IPTV_MIDDLEWARE_PRE_INIT = 10;
	public static final int IPTV_MIDDLEWARE_OPEN_URL = 11;
	public static final int IPTV_MIDDLEWARE_OPEN_HOMEPAGE = 12;
	public static final int IPTV_HIDDEN_AUTH_LOGO = 13;
	public static final int IPTV_MIDDLEWARE_OPEN_MENU = 14;

    public static final int EVENT_VLAN_DHCP_CONNECT_SUCCESSED = 11;
	public static final int EVENT_VLAN_DHCP_CONNECT_FAILED = 12;
    // TODO
    // Fixme: 这东西不能这样传！
    public static final int IPTV_MIDDLEWARE_IME_KEYBOARD = 15;
    public static final int IPTV_SOFTKEYBOARD_HEIGHT = 150;
 
	public static final int SURFACE_EPG = 1;

    public static final int STANDBY_IPTV_MODE = 0;
    public static final int EXIT_IPTV_MODE = 1;

	public static final int EVENT_RPC   = 100;

	public static final int RPC_REBOOT  = 101;
	public static final int RPC_RESTORE = 102;
	public static final int RPC_UPGRADE = 103;
	public static final int RPC_EXIT_APP = 104;
	public static final int RPC_START_APP_BYNAME = 105;
    public static final int RPC_START_APP_BYINTENT = 106;

	public static final int EVENT_PAGE   = 200;

	public static final int PAGE_AUTH_FILED = 201;
	public static final int PAGE_AUTH_FINISHED = 202;
	public static final int PAGE_AUTH_STARTED = 203;
	public static final int PAGE_LOAD_ERROR =  204;
	public static final int PAGE_LOAD_FINISHED = 205;


	public static final int IPTV_SETTINGS_TYPE_APP   = 0;
	public static final int IPTV_SETTINGS_TYPE_SYS   = 1;
	public static final int IPTV_SETTINGS_TYPE_TR069 = 2;
	public static final int IPTV_SETTINGS_TYPE_DVB   = 3;
	public static final int IPTV_SETTINGS_TYPE_MEM   = 4;

    public static final int IPTV_SETTINGS_TYPE_STRING = 0;
    public static final int IPTV_SETTINGS_TYPE_INT = 1;

	public static final int NETWORK_CONNECTTING = 0;
	public static final int NETWORK_CONNECT = 1;
	public static final int NETWORK_DISCONNECT = 2;

	public static final int VOLUME_MIN = 0;
    public static final int VOLUME_MAX = 15;

	public static final int EXTRA_EVENT_KEY = 0;
    public static final int MessageType_KeyDown = 2;
    public static final int MessageType_Char = 5;

	public final static int EIS_IRKEY_POWER = 0x0101;
	public final static int EIS_IRKEY_VOLUME_MUTE = 0x0255;
	public final static int EIS_IRKEY_NUM1 = 0x0111;
	public final static int EIS_IRKEY_NUM2 = 0x0112;
	public final static int EIS_IRKEY_NUM3 = 0x0113;
	public final static int EIS_IRKEY_NUM4 = 0x0114;
	public final static int EIS_IRKEY_NUM5 = 0x0115;
	public final static int EIS_IRKEY_NUM6 = 0x0116;
	public final static int EIS_IRKEY_NUM7 = 0x0117;
	public final static int EIS_IRKEY_NUM8 = 0x0118;
	public final static int EIS_IRKEY_NUM9 = 0x0119;
	public final static int EIS_IRKEY_NUM0 = 0x0110;
	public final static int EIS_IRKEY_CHANNEL_DOWN = 0x0252;
	public final static int EIS_IRKEY_CHANNEL_UP = 0x0251;
	public final static int EIS_IRKEY_BACK = 0x0154;
	public final static int EIS_IRKEY_UP = 0x011A;
	public final static int EIS_IRKEY_DOWN = 0x011B;
	public final static int EIS_IRKEY_LEFT = 0x011C;
	public final static int EIS_IRKEY_RIGHT = 0x011D;
	public final static int EIS_IRKEY_SELECT = 0x011E;
	public final static int EIS_IRKEY_PAGE_UP = 0x0174;
	public final static int EIS_IRKEY_PAGE_DOWN = 0x0175;
    public final static int EIS_IRKEY_AUDIO = 0x234;
	public final static int EIS_IRKEY_VOLUME_UP = 0x0253;
	public final static int EIS_IRKEY_VOLUME_DOWN = 0x0254;
	public final static int EIS_IRKEY_VK_F10 = 0x309;
	public final static int EIS_IRKEY_RED = 0x0340;
	public final static int EIS_IRKEY_GREEN = 0x0341;
	public final static int EIS_IRKEY_YELLOW = 0x0342;
	public final static int EIS_IRKEY_BLUE = 0x0343;
	public final static int EIS_IRKEY_MENU = 0x0200;
	public final static int EIS_IRKEY_REWIND = 0x0405;
	public final static int EIS_IRKEY_FASTFORWARD = 0x0404;
	public final static int EIS_IRKEY_PLAY = 0x0400;
	public final static int EIS_IRKEY_STOP = 0x0401;
	public final static int EIS_IRKEY_BTV = 0x0201;
	public final static int EIS_IRKEY_TVOD = 0x0206;
	public final static int EIS_IRKEY_VOD = 0x0205;
	public final static int EIS_IRKEY_NVOD = 0x0458;
	public final static int EIS_IRKEY_STAR = 0xCB;
	public final static int EIS_IRKEY_AUDIO_MODE = 0x0256;
	public final static int EIS_IRKEY_IME = 0x0231;
	public final static int EIS_IRKEY_INFO = 0x0237;

}
