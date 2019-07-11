/*-------------------------------------------------------------------------
 Hippo对外提供的API函数, 主要用来和现有程序过度.

-------------------------------------------------------------------------*/

#include "Hippo_api.h"
#include "Hippo_Debug.h"
#include "Hippo_Context.h"
#include "Hippo_MediaPlayer.h"


using namespace Hippo;

//c style functions.
int a_HippoContext_Init( )
{
    Hippo::HippoContext::Init( CONSTANTCONFIGFILEPATH );
    return 0;
}

int a_Hippo_API_JseRegister( const char* ioName, JseIoctlFunc rfunc, JseIoctlFunc wfunc, ioctl_context_type_e eChnl )
{
    return HippoContext::getContextInstance( )->JseRegister( ioName, rfunc, wfunc, eChnl );
}

int a_Hippo_API_UnJseRegister( const char* ioName, ioctl_context_type_e eChnl )
{
    return HippoContext::getContextInstance( )->UnJseRegister( ioName, eChnl );
}
