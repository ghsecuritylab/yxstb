#include <stdarg.h>
#include <string>
#include <vector>
#include <ctype.h>

#include <Hippo_OS.h>
#include "Hippo_HString.h"

using namespace std;
namespace Hippo
{
HString::HString() : m_str("")
{
}
HString& HString::operator= (const HString& str)
{
    m_str = str.m_str;
    return (*this);
}
HString& HString::operator= (const std::string& str)
{
    m_str = str;
    return (*this);
}
HString& HString::operator= (const char* s)
{
    m_str = s;
    return (*this);
}
HString& HString::operator= (int c)
{
    char buf[] = "FF4294967295";
    snprintf(buf, sizeof(buf), "%d", c);
    m_str = buf;
    return (*this);
}

HString& HString::operator+= (const HString& str)
{
    m_str += str.m_str;
    return (*this);
}
HString& HString::operator+= (const std::string& str)
{
    m_str += str;
    return (*this);
}
HString& HString::operator+= (const char* s)
{
    m_str += s;
    return (*this);
}
HString& HString::operator+= (char c)
{
    m_str += c;
    return (*this);
}
HString& HString::operator+= (int c)
{
    char buf[] = "FF4294967295";
    snprintf(buf, sizeof(buf), "%d", c);
    m_str += buf;
    return (*this);
}

HString& HString::format(const char *pFormat, ...)
{
    va_list args;
    char ch;
    char buffer[4096] = {0};

    va_start(args, pFormat);
    // Do the format once to get the length.
    int result = vsnprintf(&ch, 1, pFormat, args);
    // We need to call va_end() and then va_start() again here, as the
    // contents of args is undefined after the call to vsnprintf
    // according to http://man.cx/snprintf(3)
    //
    // Not calling va_end/va_start here happens to work on lots of
    // systems, but fails e.g. on 64bit Linux.
    va_end(args);
    va_start(args, pFormat);

    if(result <= 0) {
        m_str = "";
        va_end(args);
        return *this;
    }

    vsnprintf(buffer, 4096, pFormat, args);

    va_end(args);
    m_str = buffer;
    return *this;
}

/**
 * HString: ChannelID="1",ChannelName="ÐÂÎÅÓéÀÖ",UserChannelID="1",ChannelURL="igmp://238.255.100.1:10001|rtsp://110.1.1.164/PLTV/88888888/224/3221225480/00000100000000060000000000000004_0.smil?rrsip=110.1.1.164&icpid=&accounttype=1&limitflux=-1&limitdur=-1&accountinfo=:20111205180736,315,110.1.1.59,20111205180736,00000100000000050000000000000004,55E32AD3FA536225F0D0FB6DC4561598,-1,0,3,-1,,2,,,,2,END",TimeShift="1",ChannelSDP="rtsp://192.168.5.35/88888888/224/0/3221225477/00000000000g.smil?icpid=350000000001&accounttype=1&limitflux=-1&limitdur=-1&accountinfo=iptvmw",TimeShiftURL="rtsp://192.168.5.35/88888888/224/0/3221225477/00000000000g.smil?icpid=350000000001&accounttype=1&limitflux=-1&limitdur=-1&accountinfo=iptvmw",ChannelLogoStruct="",ChannelLogoURL="http://192.168.114.58/1.jpg",PositionX="450",PositionY="30",BeginTime="1",Interval="0",Lasting="10",ChannelType="1",ChannelPurchased="1"
 */
int HString::tokenize(/*out*/ vector<HString>& aKey, /*out*/ vector<HString>& aValue, int aTag)
{
    //int point = true;
    token_state_e state = TokenState_eBeforeKey;
    HString tmp;
    string::const_iterator it = m_str.begin();
    bool singalQuoteOpened = false;
    bool doubleQuotedOpend = false;
    for(; it < m_str.end(); it++) {
        if(!singalQuoteOpened && !doubleQuotedOpend && *it == aTag) {
            state = TokenState_eBeforeValue;
            aKey.push_back(tmp);
            tmp.clear();
            continue;
        } else if(!isprint(*it)) {
            continue;
        }

        if(*it == ' ') {
            switch(state) {
            case TokenState_eInKey:
                state = TokenState_eAfterKey;
                break;
            case TokenState_eInValue:
                state = TokenState_eBeforeKey;
                break;
            default:
                ;
            }
        } else if(*it == '\'') {
            if(!singalQuoteOpened) {
                switch(state) {
                case TokenState_eInValue:
                    tmp += *it;
                    break;
                case TokenState_eAfterKey:
                default:
                    break;
                }
                singalQuoteOpened = true;
            } else {
                if(doubleQuotedOpend)
                    tmp += *it;
                singalQuoteOpened = false;
            }
        } else if(*it == '\"') {
            if(!doubleQuotedOpend) {
                switch(state) {
                case TokenState_eInValue:
                    tmp += *it;
                    break;
                case TokenState_eAfterKey:
                default:
                    break;
                }
                doubleQuotedOpend = true;
            } else {
                if(singalQuoteOpened)
                    tmp += *it;
                doubleQuotedOpend = false;
            }
        } else if(*it == ',') {
            if(state == TokenState_eInValue && (singalQuoteOpened == true || doubleQuotedOpend == true))
                tmp += *it;
            else {
                aValue.push_back(tmp);
                tmp.clear();
                state = TokenState_eBeforeKey;
            }
        } else if(*it == '\r' || *it == '\n') {
            aValue.push_back(tmp);
            tmp.clear();
            state = TokenState_eBeforeKey;
        } else {
            tmp += *it;
            switch(state) {
            case TokenState_eBeforeKey:
                state = TokenState_eInKey;
                break;
            case TokenState_eBeforeValue:
                state = TokenState_eInValue;
                break;
            default:
                break;
            }
        }
    }
    aValue.push_back(tmp);
    return 0;
}

int HString::ioctlCommandSplit(HString& aInput, HString& aCmd, HString& aParam)
{
    int pos = 0;
    for(; pos < m_str.length(); pos ++) {
        if(m_str[pos])
            switch(m_str[pos]) {
            case ',':
            case ':':
                break;
            default:
                aCmd += m_str[pos];
            }
    }
    if(':' == m_str[pos]) {
        pos += 1;
    }

    aParam = m_str.substr(pos);
    return 0;
}

bool HString::equalIgnoringCase(const HString& aStr, size_t n)
{
    return equalIgnoringCase(aStr.m_str, n);
}

bool HString::equalIgnoringCase(const char* aStr, size_t n)
{
    const char* me = impl().c_str();
    if(n == 0xffffffff)
        n = impl().length();
#ifndef OS_WIN32
    return !strncasecmp(me, aStr, n);
#else
    return !_strnicmp(me, aStr, n);
    //return !lstrcmpi( me, aStr );
#endif
}
bool HString::equalIgnoringCase(const std::string& aStr, size_t n)
{
    return equalIgnoringCase(aStr.c_str(), n);
}


HString& HString::erase(size_t aStartPos, size_t aEraseNum)
{
    m_str.erase(aStartPos, aEraseNum);
    return *this;
}
HString HString::substr(const size_t off, const size_t count)
{
    string tmpStr = m_str.substr(off, count);
    return HString(tmpStr);
}

}

