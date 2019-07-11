#include <map>

#include "Hippo_OS.h"
//#include "OS/Hippo_HThreadGlobalData.h"

#include "Hippo_api.h"
#include "Hippo_Debug.h"
#include "Hippo_HString.h"
#include "Hippo_Context.h"
#include "Hippo_HBrowserActiveObj.h"
//#include "tmp/C05API_for_hippo.h"

#ifdef HUAWEI_C10
#include "./../../IPTVPlatformSpec/Huaweibase/C10/Hippo_ContextHWC10.h"
#else
#include "./../../IPTVPlatformSpec/Huaweibase/C20/Hippo_ContextHWC20.h"
#endif

namespace Hippo
{

HString HippoContext::s_NullString;
const int HippoContext::s_ioctlWriteFlag = 0;

HippoContext* HippoContext::s_pInstance = NULL;


HippoContext::HippoContext()
    : m_pMediaPlayerMgr(NULL)
    , m_BrowserAObj(NULL)
    , m_UserGroupNMB(0)
    , m_UserStatus(0)
{
    //TODO: Init Browser ActiveObj;
    m_BrowserAObj = HBrowserActiveObj::createBrowserObj();
    m_BrowserAObj->InitActiveObject();
    //globalDispatcher().setDefaultActiveObj(m_BrowserAObj);
}

HippoContext::~HippoContext()
{
    HIPPO_DEBUG("Create HippoContext Failed!\n");
}



//static
/* Hippo初始化的唯一入口函数, 创建HippoSetting, 并保存在HippoContext实例中. */
void HippoContext::Init(const char* aUrl)
{    
    if(!s_pInstance) {
#ifdef HUAWEI_C10
        s_pInstance = new HippoContextHWC10();
#else
        s_pInstance = new HippoContextHWC20();
#endif
    }
    return;
}

HippoContext* HippoContext::getContextInstance(const char* aType)
{
    if(!s_pInstance) {
        HippoContext::Init(CONSTANTCONFIGFILEPATH);
    }
    return s_pInstance;     
}

/************************************************************
* @param   aType
*
* @return
* @remark	获取播放器实例, 通常是在对应中间件Context构造函数中
*			赋值.
* @see
* @author teddy      @date 2011/01/15
************************************************************/
MediaPlayerMgr* HippoContext::getMediaPlayerMgr(const char* aType)
{
    return m_pMediaPlayerMgr;
}

int HippoContext::EPGDomain(const char* aStr)
{
    m_EPGDomain = aStr;
    return 0;
}
HString& HippoContext::EPGDomain()
{
    return m_EPGDomain;
}
HString& HippoContext::ServiceEntry(int aKey)
{
    return (HString&)s_NullString;
}
int HippoContext::EPGGroupNMB(HString& aStr)
{
    m_EPGGroupNMB = aStr;
    return 0;
}
int HippoContext::UserGroupNMB(HString& aStr)
{
    m_UserGroupNMB = aStr.toInt32();
    return 0;
}
int HippoContext::NTPDomainBackup(HString& aStr)
{
    m_NTPDomainBackup = aStr;
    return 0;
}
int HippoContext::NTPDomain(HString& aStr)
{
    m_NTPDomain = aStr;
    return 0;
}
int HippoContext::EPGDomainBackup(HString& aStr)
{
    m_EPGDomainBackup = aStr;
    return 0;
}
int HippoContext::ManagementDomainBackup(HString& aStr)
{
    m_ManagementDomainBackup = aStr;
    return 0;
}
int HippoContext::ManagementDomain(HString& aStr)
{
    m_ManagementDomain = aStr;
    return 0;
}
int HippoContext::UpgradeDomainBackup(HString& aStr)
{
    m_UpgradeDomainBackup = aStr;
    return 0;
}
int HippoContext::UpgradeDomain(HString& aStr)
{
    m_UpgradeDomain = aStr;
    return 0;
}

//virtual
int HippoContext::AuthenticationCTCGetAuthInfo(HString& aToken, HString& aResult)
{
    return 0;
}

int HippoContext::AuthenticationCTCSetConfig(HString& aField, HString& aValue)
{
    if(!aField.equalIgnoringCase("Channel"))
        HIPPO_DEBUG("fieldName=%s,fieldValue=%s\n", aField.c_str(), aValue.c_str());

    return ioctlWrite(aField, aValue);
}

int HippoContext::AuthenticationCTCGetConfig(HString& aField, HString& aValue)
{
    int rval = -1;
    
    rval = ioctlRead(aField, aValue);
    HIPPO_DEBUG("fieldName=%s,fieldValue=%s\n", aField.c_str(), aValue.c_str());
    return rval;
}

int HippoContext::JseRegister(const char *ioName, JseIoctlFunc &rfunc, JseIoctlFunc &wfunc, ioctl_context_type_e eChnl)
{
    return 0;
}

int HippoContext::UnJseRegister(const char *ioName, ioctl_context_type_e eChnl)
{
    return 0;
}

int HippoContext::ioctlRead(HString &aField, HString &aValue)
{
    char buf[2048] = { 0 };
    int rval = -1;

    aValue += buf;
    return rval;
}

int HippoContext::ioctlWrite(HString &aField, HString &aValue)
{
    return -1;
}

//以下为和hippo glue保持一致，预留
void
HippoContext::Init()
{
}

}


