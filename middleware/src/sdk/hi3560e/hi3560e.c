#if defined(hi3560e)
extern int io_input_IRtype(void);

int ymm_decoder_getDolbyDownmixingStatus(int* status)
{
    return -1;    
}

int yhw_vout_setHDMIAudioSelfAdaption(int mode)
{
   return -1;    
}

int yhw_aout_setSpdifOutputMode(int mode)
{
    return -1;    
} 

int yhw_aout_setHDMIAudioMode(int mode)
{
    return -1;    
}

int yhw_vout_setHdcpFailureMode(int mode)
{
    return -1;    
}

int yhw_vout_getHDCPStatus(int* status)
{
    return -1;    
}

int yhw_board_getHWVersion(unsigned int* hardwareVer)
{
    if (hardwareVer)
        *hardwareVer = 0x300;
    return 0;    
}

int mid_stream_set_apple_buffersize(int size)
{
    return -1;    
}

int yhw_env_getSystemMemorySize(int* size)
{
    return -1;    
}

int yhw_env_setSystemMemorySize(int* size)
{
    return -1;    
}
 
int Hybroad_Inconsistent_getIRtype()
{
    int ret = 0;
    if (1 != io_input_IRtype())
        ret = 1; 
    return ret;  
}

char* Hybroad_getPlantformDesc()
{
    return "Hi3560E";    
}

char* Hybroad_getHWtype()
{
    return "3560E";        
} 
#endif