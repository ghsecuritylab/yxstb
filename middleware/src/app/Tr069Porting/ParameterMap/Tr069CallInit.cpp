
#include "Tr069CallInit.h"

#include "Tr069Call.h" 
#include "Tr069Root.h"

#include "Device/Tr069Device.h"


/*************************************************
Description: ��ʼ��Tr069����Ľӿ�,������root��
Input: ��
Return: ��
 *************************************************/
extern "C" int Tr069CallInit()
{

    /* ע��root.Device �����±� */
    Tr069Call* dev = new Tr069Device();
    Tr069RootRegist(dev->name(), dev);
	
	
    return 0;
}
