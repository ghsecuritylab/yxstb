
#include "JseRoot.h"

#include "JseGroupCall.h"
#include "Huawei/JseHuawei.h"
#include "Hybroad/JseHybroad.h"

static JseGroupCall g_root("root");

/*************************************************
Description: ����ڵ������һ���ӽڵ�
Input:  name: �ӽڵ���
        call: �ӽڵ�Ľṹָ��
Return: 0 Ϊ�ɹ�ע��
 *************************************************/
int JseRootRegist(const char *name, JseCall *call)
{
    return g_root.regist(name, call);
}

/*************************************************
Description: ���ӿ�����ȡ�����ڵĸ��ڵ�ָ��
Input:  name: �ӿ���
Return: 0Ϊ�ɹ�����
 *************************************************/
JseGroupCall* getNodeByName(const char* name)
{
    return (JseGroupCall*)g_root.getNode(name);
}

/*************************************************
Description: Jse ע���ӿڣ���δʵ��
Input:  name: Ҫע���Ľӿ���
Return: 0Ϊ�ɹ�����
 *************************************************/
JseCall *JseRootUnregist(const char *name)
{
    return 0;
}

/*************************************************
Description: Jse ���ö��ӿڵ���ں���
Input:  name:   ��������
        param:  ����Ĳ���
        value:  ���ص�buffer
        length: buffer����
Return: 0Ϊ�ɹ�����
        ����ֵ����
 *************************************************/
extern "C" int JseRootRead(const char *name, const char *param, char *value, int length)
{
    return g_root.call(name, param, value, length, 0);
}

/*************************************************
Description: Jse ���ö��ӿڵ�д�ں���
Input:  name:   ��������
        param:  ����Ĳ���
        value:  Ҫд���buffer
        length: ���ã�д0����
Return: 0Ϊ�ɹ�����
        ����ֵ����
 *************************************************/
extern "C" int JseRootWrite(const char *name, const char *param, char *value, int length)
{
    return g_root.call(name, param, value, length, 1);
}

/*************************************************
Description: Jse ��ʼ���ӿ�
Input:  ��
Return: 0Ϊ�ɹ�����
 *************************************************/
extern "C" int JseRootInit()
{
    JseHuaweiInit();
    JseHybroadInit();
    return 0;
}
