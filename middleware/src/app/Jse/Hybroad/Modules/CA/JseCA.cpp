
#include "JseCA.h"

#if defined(INCLUDE_VMCA)
#include "Verimatrix/JseVerimatrix.h"
#endif

/*************************************************
Description: ��ʼ��CA����Ľӿ�
Input: ��
Return: 0
 *************************************************/
int JseCAInit()
{
#if defined(INCLUDE_VMCA)
   JseVerimatrixInit();
#endif
    return 0;
}
