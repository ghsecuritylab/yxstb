
#ifndef __TR069_PORT_CTC_H__
#define __TR069_PORT_CTC_H__

enum {
	FLAG_AUTHNUM,
	FLAG_AUTHFAILNUM,
	FLAG_MULTIREQNUM,
	FLAG_VODREQNUM,
	FLAG_MULTIFAILNUM,
	FLAG_VODFAILNUM,
	FLAG_BUFINCNUM,
	FLAG_BUFDECNUM
};

enum {
	TYPE_VOD,
	TYPE_CHANNEL
};
void monitor_cycle_statistics(int flag);

int app_tr069_port_Monitor_get_AuthNumbers(void);


int app_tr069_port_Monitor_get_AuthFailNumbers(void);

int app_tr069_port_Monitor_get_MultiReqNumbers(void);

int app_tr069_port_Monitor_get_VodReqNumbers(void);

int app_tr069_port_Monitor_get_MultiFailNumbers(void);

int app_tr069_port_Monitor_get_VodFailNumbers(void);
int app_tr069_port_Monitor_get_BufUnderFlowNumbers(void);
int app_tr069_port_Monitor_get_BufOverFlowNumbers(void);
int monitor_post_play(int type, int delay, const char *channelName, const char *channelAddress);
int app_tr069_port_Monitor_get_ResponseDelay(void);
int app_tr069_port_Monitor_get_ChannelSwitchDelay(void);


#endif//__TR069_PORT_CTC_H__
