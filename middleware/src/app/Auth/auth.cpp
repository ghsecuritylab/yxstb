

#include "auth.h"
#include "default_eds_roller.h"
#include "default_url_checker.h"
#include "concatenator_hw_auth.h"
#include "concatenator_c20_auth.h"
#include "SysSetting.h"
#include "AppSetting.h"

namespace Hippo {

Auth::EDS::EDS()
    : m_roller(new DefaultEdsRoller())
    , m_checker(new DefaultUrlChecker())
{
}

Auth::EDS::EDS(Auth::EdsRoller* r, Auth::UrlChecker* uc, Auth::Concatenator* c)
    : m_roller(r)
    , m_checker(uc)
    , m_concatenator(c)
{
    m_roller->safeRef();
    m_checker->safeRef();
    m_concatenator->safeRef();
}

Auth::EDS::~EDS()
{
    m_roller->safeUnref();
    m_checker->safeUnref();
    m_concatenator->safeUnref();
}

void Auth::EDS::Reset()
{
    // Reset: 回到第一个可用URL
    if (m_roller) {
        m_roller->Reset();
        std::string url = Current();
        if (url.empty()) {
            Next();
        }
    }
}

void Auth::EDS::Next()
{
    // Next: 跳到下一个可用URL
    if (m_roller) {
        std::string url;
        while (!(*m_checker)(url) && !m_roller->IsEnd()) {
            m_roller->Next();
            url = m_roller->Current();
        }
    }
}

std::string Auth::EDS::Current()
{
    if (m_roller) {
        std::string url = m_roller->Current();
        if (m_checker) {
            if (!(*m_checker)(url)) {
                return "";
            }
            if (m_concatenator) {
                return (*m_concatenator)(url);
            }
            return url;
        }
        return url;
    }
    return "";
}

Auth::Auth(Auth::EdsRoller* r, UrlChecker* uc, Concatenator* c)
    : m_checker(uc)
    , m_concatenator(c)
    , m_eds(r, uc, c)
{
    m_checker->safeRef();
    m_concatenator->safeRef();
}

Auth::Auth()
    : m_checker(new DefaultUrlChecker())
#if defined(HUAWEI_C10)
    , m_concatenator(new ConcatenatorHwAuth())
#else
    , m_concatenator(new ConcatenatorC20Auth())
#endif
    , m_eds()
{
}

Auth::~Auth()
{
    m_checker->safeUnref();
    m_concatenator->safeUnref();
}

std::string Auth::Get(AuthVar key)
{
    std::map<AuthVar, std::string>::iterator    it;
    it = m_AuthMap.find(key);
    if (it != m_AuthMap.end())
        return it->second;
    return std::string("");
}

void Auth::Set(AuthVar key, std::string val)
{
    m_AuthMap[key] = val;
}

std::string Auth::AvailableEpgUrl()
{
    char temp[4096] = {0};
    std::string url;
    url = Get(eAuth_epgDomain);
    if (!CheckUrl(url)) {
        url = Get(eAuth_epgDomainBackup);
    }
    if (!CheckUrl(url)) {
        url = eds().Current();
    }
    if (!CheckUrl(url)) {
        sysSettingGetString("eds", temp, sizeof(temp), 0);
        url = temp;
    }
    if (!CheckUrl(url)) {
        sysSettingGetString("eds1", temp, sizeof(temp), 0);
        url = temp;
    }
    if (!CheckUrl(url)) {
        url = "";
    }
    return url;
}

std::string Auth::AvailableEpgAuthUrl()
{
    std::string url = AvailableEpgUrl();
    if (m_concatenator)
        return (*m_concatenator)(url);
    return url;
}

std::string Auth::AvailableEpgUrlWithoutPath()
{
    std::string url = AvailableEpgUrl();
    if (url.empty())
        return url;
    std::string::size_type pos = url.find_first_of("/", 8);
    if (pos != std::string::npos) {
        url = url.substr(0, pos);
    }
    url += std::string("/");
    return url;
}

bool Auth::CheckUrl(std::string url)
{
    if (m_checker) {
        return (*m_checker)(url);
    }
    if (url.empty())
        return false;
    if (url.substr(0, 7) != "http://" && url.substr(0, 8) != "https://")
        return false;
    return true;
}

std::string Auth::ConcatenateUrl(std::string url)
{
    if (m_concatenator && CheckUrl(url)) {
        return (*m_concatenator)(url);
    }
    return url;
}


} // namespace Hippo





