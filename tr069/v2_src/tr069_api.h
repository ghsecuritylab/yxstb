/*******************************************************************************
	��˾��
			Yuxing software
	��¼��
			2008-1-26 21:12:26 create by Liu Jianhua
	ģ�飺
			tr069
	������
			TR069�ӿ�
 *******************************************************************************/
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/*------------------------------------------------------------------------------
	TR069��ʼ��
 ------------------------------------------------------------------------------*/
int tr069_api_init(void);

/*------------------------------------------------------------------------------
	������Щ�����ý���TR069��ؽӿ�
 ------------------------------------------------------------------------------*/
/*
    name
    1.Device.x��������������
    2.Config.
        Save�����ñ���
        Reset����������

        Huawei��15��C15�淶��, 58��C58�淶����Ĭ�Ͼ�������
        Bootstrap����һ����ACS�Ự�Ƿ�Я��BOOTSTRAP�¼���Ĭ��YES
        DigestAuth���Ƿ��ACS֪ͨ����Digest��֤��Ĭ��YES
        ParamPedant���Ƿ���ϸ���SetParameterValues�������ͣ�Ĭ��NO
        HoldCookie���Ƿ񱣳ֻỰ��Session��Cookie��Ĭ��NO
        STUNUsername��STUN needUserNameAttr
        ParamPath�������ļ�·��

    3.Task.
        Active�����񼤻�
        Suspend���������
        Connect����������Ự

    4.Event.
        ValueChange�������ı䣬strΪ������������
        AddObject��strΪ��Object��������Object��instance
        DeleteObject��strɾ��Object

        Reigst��ע���¼���val���¼���ţ�strΪ�¼�����
        Param�����¼���ʱ��Ҫ�ϱ��Ĳ�����val���¼���ţ�strΪ������������
        Post���¼��ϱ���val���¼���ţ�strΪ�¼��ϱ��ɹ��ش����������ڻش���������
        Downloadt�����ؽ����str�������ͣ�val
        Upload���ϴ��ؽ����str�������ͣ�val
        List���г�����������ʹ��
        Shutdown��������������ʹ��
 */
int tr069_api_setValue(char *name, char *str, unsigned int val);
/*
    name
    1.Device.x��������������
 */
int tr069_api_getValue(char *name, char *str, unsigned int val);


int tr069_api_registFunction(char *, int (*)(char *, char *, unsigned int), int (*)(char *, char *, unsigned int));
int tr069_api_registOperation(char *, int (*)(char *, char *, unsigned int));

#ifdef __cplusplus
}
#endif

