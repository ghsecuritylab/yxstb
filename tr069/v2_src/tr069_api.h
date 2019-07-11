/*******************************************************************************
	公司：
			Yuxing software
	纪录：
			2008-1-26 21:12:26 create by Liu Jianhua
	模块：
			tr069
	简述：
			TR069接口
 *******************************************************************************/
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/*------------------------------------------------------------------------------
	TR069初始化
 ------------------------------------------------------------------------------*/
int tr069_api_init(void);

/*------------------------------------------------------------------------------
	下面这些是配置界面TR069相关接口
 ------------------------------------------------------------------------------*/
/*
    name
    1.Device.x：参数完整名称
    2.Config.
        Save：配置保存
        Reset：配置重置

        Huawei：15（C15规范）, 58（C58规范），默认均不开启
        Bootstrap：第一次与ACS会话是否携带BOOTSTRAP事件，默认YES
        DigestAuth：是否对ACS通知进行Digest认证，默认YES
        ParamPedant：是否对严格检测SetParameterValues参数类型，默认NO
        HoldCookie：是否保持会话（Session）Cookie，默认NO
        STUNUsername：STUN needUserNameAttr
        ParamPath：参数文件路径

    3.Task.
        Active：任务激活
        Suspend：任务挂起
        Connect：主动发起会话

    4.Event.
        ValueChange：参数改变，str为参数完整名称
        AddObject：str为父Object，返回新Object的instance
        DeleteObject：str删除Object

        Reigst：注册事件，val是事件序号，str为事件名称
        Param：给事件临时绑定要上报的参数，val是事件序号，str为参数完整名称
        Post：事件上报，val是事件序号，str为事件上报成功回传参数，用于回传附件参数
        Downloadt：下载结果：str下载类型，val
        Upload：上传载结果：str上载类型，val
        List：列出参数，调试使用
        Shutdown：：待机，调试使用
 */
int tr069_api_setValue(char *name, char *str, unsigned int val);
/*
    name
    1.Device.x：参数完整名称
 */
int tr069_api_getValue(char *name, char *str, unsigned int val);


int tr069_api_registFunction(char *, int (*)(char *, char *, unsigned int), int (*)(char *, char *, unsigned int));
int tr069_api_registOperation(char *, int (*)(char *, char *, unsigned int));

#ifdef __cplusplus
}
#endif

