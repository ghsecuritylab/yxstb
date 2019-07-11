#include "stdio.h"
#include "libzebra.h"
#include "Assertions.h"

#if defined(hi3716m)

char* Hybroad_getPlantformDesc()
{
    return "Hi3716M";    
}

char* Hybroad_getHWtype()
{
    return "3716M";        
}

int Hybroad_Inconsistent_getIRtype()
{
    int ret = 0;
    if (!yhw_input_IRtype())
        ret = 1;  
    return ret;  
}

#endif