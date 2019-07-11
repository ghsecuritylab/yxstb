/*******************************************************************************
	公司：
			Yuxing software
	纪录：
			2008-1-26 21:12:26 create by Liu Jianhua
	模块：
			tr069
	简述：
			导入接口
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
    参数注册
 */
void tr069_port_infoPath(char *path);

/*
    获取配置
 */
int tr069_port_infoLoad(char *buf, int len);
/*
    保存配置
 */
int tr069_port_infoSave(char *buf, int len);

/*
    模块扩展初始化
 */
void tr069_port_moduleInit(void);

/*
    通过用户自定义错误码获取错误描述

    faultcode：用户自定义错误码
    faulstring：用户自定义错误码对应的错误描述
 */
int tr069_port_fault2string(int faultcode, char *faulstring, int maxlen);

/*
    获取默认配置
 */
int tr069_port_getDefault(char *buf, int len);

/*
    获取参数
 */
int tr069_port_getValue(char *name, char *str, unsigned int val);
/*
    设置参数
 */
int tr069_port_setValue(char *name, char *str, unsigned int val);

#ifdef __cplusplus
}
#endif

#endif //__TR069_PORT_H__

