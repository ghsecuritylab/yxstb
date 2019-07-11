
#include "Hippo_OS.h"
#include "Hippo_Debug.h"

#include "Hippo_ContextHWC10.h"

#include "JseRoot.h"

namespace Hippo {

HippoContextHWC10::HippoContextHWC10()
{
    m_pMediaPlayerMgr = &m_MediaPlayerMgr;
}

HippoContextHWC10::~HippoContextHWC10()
{
}

int
HippoContextHWC10::JseRegister(const char *ioName, JseIoctlFunc &rfunc, JseIoctlFunc &wfunc, ioctl_context_type_e eChnl)
{
    ioctlMapNode<JseIoctlFunc> mapNode(rfunc, wfunc);

    if(eChnl != IoctlContextType_eHWBaseC20)
        m_ioctlCustomMap[ioName] = mapNode;
    return 0;
}

int
HippoContextHWC10::UnJseRegister(const char *ioName, ioctl_context_type_e eChnl)
{
    if(eChnl != IoctlContextType_eHWBaseC20)
        m_ioctlCustomMap.erase(ioName);
    return 0;
}

int
HippoContextHWC10::ioctlRead(HString& aField, HString& aValue /*out*/)
{
    HString param;
    HString newField;
    int ret = -1;
    char buf[4096] = { 0 };
    const char *start = NULL;
    const char *cmdend = NULL;
    const char *parastart = NULL;
    const char *str;
    char c;

    HIPPO_DEBUG("ioctlRead %s\n", aField.c_str());
    //TODO: 准备运行参数
    str = aField.c_str();
    start = str;
    GET_CHAR(c, str);
    while(c != '\0') {
        switch(c) {
        case ',': {
            cmdend = str - 1;
            parastart = str;
            goto end;
        }
        case ':':
        case '^': {
            GET_CHAR(c, str);
            if(c == ':') {
                cmdend = str - 2;
                parastart = str;
            } else {
                cmdend = str - 2;
                parastart = str - 1;
            }
            goto end;
        }
        default: {
            break;
        }
        }
        GET_CHAR(c, str);
    }
end:
    if(cmdend != NULL && parastart != NULL) {
        char newFieldstr[4096] = {0};
        char parastr[4096] = {0};

        strncpy(newFieldstr, start, cmdend - start);
        newFieldstr[cmdend - start] = '\0';
        strcpy(parastr, parastart);
        newField = newFieldstr;
        param = parastr;
    } else {
        newField = aField;
    }
    //TODO: 准备运行参数
    HIPPO_DEBUG("ioctlRead %s<->%s\n", newField.c_str(), param.c_str());

    ret = JseRootRead(newField.c_str(), param.c_str(), buf, 4096);
    if(!ret){
         aValue = buf;
        return ret;
    }

    do {
        //搜索客户注册列表
        JseIoctlFunc pCustomFunc = 0;
        ioctlCustomMap::const_iterator itc;
        itc = m_ioctlCustomMap.find(newField.c_str());
        if(itc != m_ioctlCustomMap.end()) {
            const ioctlMapNode<JseIoctlFunc>& ioctlNode = (itc->second);
            if(ioctlNode.m_ioctlRead != 0) {
                ret = (ioctlNode.m_ioctlRead)(newField.c_str(), param.c_str(), buf, 4096);
                break;
            }
        }
        ret = HippoContextHWBase::ioctlRead(aField, aValue);
        if(!ret)
            return ret;
    } while(0);
    aValue = buf;
    return ret;
}

int
HippoContextHWC10::ioctlWrite(HString &aField, HString &aValue /*in*/)
{
    HString param;
    int ret = -1;

    HIPPO_DEBUG("ioctlWrite %s:%s\n", aField.c_str(), aValue.c_str());
    ret = JseRootWrite(aField.c_str(), NULL, (char *)aValue.c_str(), 0);
    if(!ret)
        return ret;

    do {
        JseIoctlFunc pCustomFunc = 0;
        ioctlCustomMap::const_iterator itc;
        itc = m_ioctlCustomMap.find(aField.c_str());

        if(itc != m_ioctlCustomMap.end()) {
            const ioctlMapNode<JseIoctlFunc>& ioctlNode = (itc->second);
            if(ioctlNode.m_ioctlWrite != 0) {
                ret = (ioctlNode.m_ioctlWrite)(aField.c_str(), param.c_str(), (char*)aValue.c_str(), 0);
                break;
            }
        }
        ret = HippoContextHWBase::ioctlWrite(aField, aValue);
        if(!ret)
            return ret;
    } while(0);
    return ret;
}

int
HippoContextHWC10::AuthenticationCTCGetAuthInfo(HString& aToken, HString& aResult)
{
    int rval = -1;

    HString str("EncryToken");
    rval = ioctlWrite(str, aToken);
    str = "AuthenticatorCTC_Encry";
    rval = ioctlRead(str, aResult);
    return rval;
}

int
HippoContextHWC10::AuthenticationCTCSetConfig(HString& aField, HString& aValue)
{
    if(!aField.equalIgnoringCase("Channel"))
        HIPPO_DEBUG("fieldName=%s,fieldValue=%s\n", aField.c_str(), aValue.c_str());
    else
        HIPPO_DEBUG("fieldName=%s\n", aField.c_str());
    return ioctlWrite(aField, aValue);
}

int
HippoContextHWC10::AuthenticationCTCGetConfig(HString& aField, HString& aValue)
{
    HIPPO_DEBUG("fieldName=%s, fieldValue=%s\n", aField.c_str(), aValue.c_str());
    return ioctlRead(aField, aValue);
}

int
HippoContextHWC10::AuthenticationCUGetAuthInfo(HString& aToken, HString& aResult)
{
    HString str("EncryToken");
    if(ioctlWrite(str, aToken) < 0)
        return -1;
    str = "AuthenticatorCU_Encry";
    return ioctlRead(str, aResult);
}

int
HippoContextHWC10::AuthenticationCUSetConfig(HString& aField, HString& aValue)
{
    return ioctlWrite(aField, aValue);
}

int
HippoContextHWC10::AuthenticationCUGetConfig(HString& aField, HString& aValue)
{
    return ioctlRead(aField, aValue);
}

}
