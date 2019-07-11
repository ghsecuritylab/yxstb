#ifndef __HIPPO_HippoContextHWC10_API_H
#define __HIPPO_HippoContextHWC10_API_H

#include <map>

#include <Hippo_HString.h>
#include <Hippo_Context.h>

#include "../Hippo_ContextBase.h"

#include "BrowserPlayerManager.h"
#include "Huawei/C10/BrowserPlayerManagerC10.h"

namespace Hippo
{

class HippoContextHWC10 : public HippoContextHWBase
{
    typedef int (HippoContextHWC10::*ioctlFunc)(HString& , HString&, HString&, int);
    typedef std::map<const HString, ioctlMapNode<ioctlFunc> > ioctlMap;
public:
    HippoContextHWC10();
    ~HippoContextHWC10();

    virtual int JseRegister(const char* ioName, JseIoctlFunc& rfunc, JseIoctlFunc& wfunc, ioctl_context_type_e eChnl);
    virtual int UnJseRegister(const char* ioName, ioctl_context_type_e eChnl);
    virtual int ioctlWrite(HString& aField, HString& aValue /*in*/);
    virtual int ioctlRead(HString& aField, HString& aValue /*out*/);

    //interface corresponding to CTC IPTV2.0 /HW 2.0 Authentication::CTCSetConfig
    virtual int AuthenticationCTCSetConfig(HString& aField, HString& aValue);
    virtual int AuthenticationCTCGetConfig(HString& aField, HString& aValue);
    virtual int AuthenticationCTCGetAuthInfo(HString& aToken, HString& aResult);
    virtual int AuthenticationCUSetConfig(HString& aField, HString& aValue);
    virtual int AuthenticationCUGetConfig(HString& aField, HString& aValue);
    virtual int AuthenticationCUGetAuthInfo(HString& aToken, HString& aResult);

private:
    BrowserPlayerManagerC10 m_MediaPlayerMgr;

private:
    ioctlCustomMap m_ioctlCustomMap;
};

}

#endif

