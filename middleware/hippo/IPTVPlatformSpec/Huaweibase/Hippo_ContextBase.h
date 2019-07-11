#ifndef __HIPPO_HippoContextHWBase_API_H
#define __HIPPO_HippoContextHWBase_API_H

#include <map>

#include <Hippo_HString.h>
#include <Hippo_Context.h>
#include <Hippo_api.h>


namespace Hippo
{

#define GET_CHAR(dest,str)  dest=*str++

class HippoContextHWBase : public HippoContext
{
    typedef int (HippoContextHWBase::*ioctlFunc)(HString& aFieldName, HString& aFieldParam, HString& aFieldValue, int aResult);
    typedef std::map<const HString, ioctlMapNode<ioctlFunc> > ioctlMap;

public:
    HippoContextHWBase();
    ~HippoContextHWBase();

    virtual int JseRegister(const char* ioName, JseIoctlFunc& rfunc, JseIoctlFunc& wfunc, ioctl_context_type_e eChnl) { return 0; }
    virtual int UnJseRegister(const char* ioName, ioctl_context_type_e eChnl) { return 0; }
    //intreface corresponding to iPanel.ioctlWrite/ioctlRead.
    virtual int ioctlWrite(HString& aField, HString& aValue /*in*/);
    virtual int ioctlRead(HString& aField, HString& aValue /*out*/);

private:
    int ioctl_printf(HString& aField, HString& aParam, HString& aValue, int aResult);

private:
    ioctlMap m_ioctlMap;
};

}

#endif

