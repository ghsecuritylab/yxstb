
#include "JseCA.h"

#if defined(INCLUDE_VMCA)
#include "Verimatrix/JseVerimatrix.h"
#endif

/*************************************************
Description: 初始化CA定义的接口
Input: 无
Return: 0
 *************************************************/
int JseCAInit()
{
#if defined(INCLUDE_VMCA)
   JseVerimatrixInit();
#endif
    return 0;
}
