#ifndef __APP_EPG_PARA_H__
#define __APP_EPG_PARA_H__


#define TR069_UPGRADE_REQUEST 0x9204 //reconfirm
#define TR069_NET_CONNECT_OK 0x9205
#define TR069_UPGRADE_AUTO 0x920A //C27
#define TR069_UPGRADE_TMS 0x920B //C27
#define UPGRADE_RELOCATION 0x920C //C27 ��������Ϣ������õ������Ƿ���TMS�ӹܣ�������ӹ�ѭ�����
#define TR069_UPGRADE_BOOT_REQUEST 0x920D //C27 ��������Ϣ������õ������Ƿ���TMS�ӹܣ�������ӹ�ѭ�����
#define TR069_NET_CONNECT_ERROR 0x9210	//C27
#define TR069_MSG_START 0x924E	//C27
#define TR069_MSG_STOP 0x924F	//C27


#ifdef __cplusplus
extern "C" {
#endif

    void app_Init(void);

    void app_stbmonitor_tms_url_set(const char* url);
    int app_stbmonitor_tms_url_get(char *buf);

#ifdef __cplusplus
}
#endif

#endif
