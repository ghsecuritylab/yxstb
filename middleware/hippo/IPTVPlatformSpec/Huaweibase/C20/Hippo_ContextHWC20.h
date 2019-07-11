#define __HIPPO_HippoContextHWC20_API_H

#include <map>

#include <Hippo_HString.h>

#include "../Hippo_ContextBase.h"

#include "BrowserPlayerManager.h"
#include "Huawei/C20/BrowserPlayerManagerC20.h"

namespace Hippo{

class HippoContextHWC20 : public HippoContextHWBase
{
public:
    HippoContextHWC20();
    ~HippoContextHWC20();

    virtual int JseRegister(const char* ioName, JseIoctlFunc& rfunc, JseIoctlFunc& wfunc, ioctl_context_type_e eChnl);
    virtual int UnJseRegister(const char* ioName, ioctl_context_type_e eChnl);
    virtual int ioctlWrite(HString& aField, HString& aValue /*in*/);
    virtual int ioctlRead(HString& aField, HString& aValue /*out*/);

    virtual int AuthenticationCTCGetAuthInfo(HString& aToken, HString& aResult);
    virtual int AuthenticationCUSetConfig(HString& aField, HString& aValue) { return -1; }
    virtual int AuthenticationCUGetConfig(HString& aField, HString& aValue) { return -1; }
    virtual int AuthenticationCUGetAuthInfo(HString& aToken, HString& aResult) { return -1; }
    
protected:
    BrowserPlayerManagerC20 m_MediaPlayerMgr;

private:
    ioctlCustomMap m_ioctlCustomMap;
};

}
