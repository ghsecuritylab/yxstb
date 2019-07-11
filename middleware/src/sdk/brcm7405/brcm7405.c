#include "stdio.h"
#include "libzebra.h"
#include "Assertions.h"

#if defined(brcm7405)

int ymm_decoder_getDolbyDownmixingStatus(int* status)
{
    return -1;    
}

int yhw_vout_setHDMIAudioSelfAdaption(int mode)
{
    return -1;    
}

int yhw_vout_setHdcpFailureMode(int mode)
{
    return -1;    
}

int yhw_board_getHWVersion(unsigned int* hardwareVer)
{
    return -1;    
}

int mid_stream_set_apple_buffersize(int size)
{
    return -1;    
}

int yhw_env_getSystemMemorySize(int* size)
{
    return -1;    
}

int Hybroad_Inconsistent_getIRtype()
{
    int ret = 0; 
    return ret;  
}

char* Hybroad_getPlantformDesc()
{
    return "BCM7406";
}

char* Hybroad_getHWtype()
{
    return "7405";        
}

#endif //defined(brcm7405) 