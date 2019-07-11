#include "Tr069Root.h"

#include "Tr069Call.h"
#include "Tr069GroupCall.h"
#include "Device/Tr069Device.h"

#include "Tr069CallInit.h"
#include "TR069Assertions.h"

static Tr069GroupCall g_root("root"); /*Tr069���нӿڵĸ��ڵ�*/

/*************************************************
Description: Tr069����ڵ�ע���ӽڵ�Ľӿ�
Input:  ��
Return: 0�ɹ�������ʧ��
 *************************************************/
int Tr069RootRegist(const char *name, Tr069Call *call)
{
    return g_root.regist(name, call);
}

/*************************************************
Description: Tr069�ڵ�ע��ӿ�
Input:  ��
Return: ��
 *************************************************/
extern "C" int Tr069RootInit()  // �ڴ���ӽڵ��ʼ������
{
    Tr069CallInit();  //tanf
   
    return 0;     
}

/*************************************************
Description: Tr069 ע���ӿڣ���δʵ��
Input:  name: Ҫע���Ľӿ���
Return: 0Ϊ�ɹ�����
 *************************************************/
Tr069Call* Tr069RootUnregist(const char *name)
{
    return 0;
}

/*************************************************
Description: Tr069���ö��ӿ������ַ��ط�ʽ��
             1������һ��buf
             2��������ֵ��ֻ�õ�pval������ת������ʹ��JseCall�Ľṹ
Input:  name:   ��������
        str: ����buffer���׵�ַ
        pval:����buffer�ĳ��ȵ�ָ��
Return: ��
 *************************************************/
extern "C" int Tr069RootRead(const char *name, char *str, unsigned int val) 
{ 
    int ret = -1;
    if (str) { 
        str[0] = '\0'; // �ȸ���Ĭ��ֵ
        
         ret =  g_root.call(name, str, val, 0);
	  LogTr069Debug("tr069 read iptv[%s] ------[%s], ret = %d\n",name, str, ret);
	  return ret;
    }
    LogTr069Error("name ------[%s] str is null,will  ret = -1\n",name);

	return -1;
} 
  
/*************************************************
Description: Tr069����д�ӿ�
Input:  name: ��������
        str:  Ҫд��buffer���׵�ַ
        pval: buffer�ĳ��ȵ�ָ��
Return: ��
 *************************************************/
extern "C" int Tr069RootWrite(const char *name, char *str, unsigned int val) 
{ 
    char buf[10] = "0";
    int ret = -1;	
    if (str) {
        ret = g_root.call(name, str, val, 1);
	 LogTr069Debug("tr069 write iptv[%s] ------[%s], ret = %d\n",name, str, ret);
        return ret;
    }
    LogTr069Error("name ------[%s] str is null,will  ret = -1\n",name);

    return -1;
} 
