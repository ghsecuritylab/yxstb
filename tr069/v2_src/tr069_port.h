/*******************************************************************************
	��˾��
			Yuxing software
	��¼��
			2008-1-26 21:12:26 create by Liu Jianhua
	ģ�飺
			tr069
	������
			����ӿ�
 *******************************************************************************/
#ifndef __TR069_PORT_H__
#define __TR069_PORT_H__

typedef enum
{
	UPGRADE_CONNECT_SERVER_FAIL = 9810,
	UPGRADE_NET_DISCONNECT,
	UPGRADE_ILLEGAL_VERSION,
	UPGRADE_WRITE_FLASH_FAIL,
	UPGRADE_SAME_VERSION_AS_SERVERS,
	UPGRADE_IS_RUNNING_ALREADY,
}TR069_UPGRADE_ERROR_CODE;

#ifdef __cplusplus
extern "C" {
#endif

/*
    ����ע��
 */
void tr069_port_infoPath(char *path);

/*
    ��ȡ����
 */
int tr069_port_infoLoad(char *buf, int len);
/*
    ��������
 */
int tr069_port_infoSave(char *buf, int len);

/*
    ģ����չ��ʼ��
 */
void tr069_port_moduleInit(void);

/*
    ͨ���û��Զ���������ȡ��������

    faultcode���û��Զ��������
    faulstring���û��Զ���������Ӧ�Ĵ�������
 */
int tr069_port_fault2string(int faultcode, char *faulstring, int maxlen);

/*
    ��ȡĬ������
 */
int tr069_port_getDefault(char *buf, int len);

/*
    ��ȡ����
 */
int tr069_port_getValue(char *name, char *str, unsigned int val);
/*
    ���ò���
 */
int tr069_port_setValue(char *name, char *str, unsigned int val);

#ifdef __cplusplus
}
#endif

#endif //__TR069_PORT_H__

