#include "JseGlueAssertions.h"
#include "IUBankDeviceCell.h"
#include "Hippo_Context.h"
#include "Hippo_HString.h"

#include <stdio.h>
 
using namespace Hippo;

IUBankDeviceCell::IUBankDeviceCell()
    : m_device(NULL)
{
    LogJseGlueDebug("new\n");
}

int 
IUBankDeviceCell::createUBankDeviceInstance(const char* name)
{
    LogJseGlueDebug("name<%s>\n", name);

    if(!m_device)
	    m_device = new Hippo::CTCUBankDevice(NULL, name); 
	if (!m_device)
        return -1;
    return 0;
}

int 
IUBankDeviceCell::set(const char *ioStr, const char *wrStr)
{
    LogJseGlueDebug("str<%s , %s>\n", ioStr, wrStr);

    HString ioctlStr = ioStr;
    HString ioctlValue = wrStr;

    if (m_device)
        m_device->set(ioctlStr, ioctlValue);

    return 0;
}

int 
IUBankDeviceCell::get(const char *ioStr, char* wrStr)
{
    int ret = -1;
    HString ioctlStr = ioStr;

    wrStr[0] = '\0';
	if (m_device) {
		const char* pStr = m_device->get(ioctlStr);
        if (pStr) {
            strncpy(wrStr, pStr, IJSEBUFFSIZE);
            wrStr[IJSEBUFFSIZE] = 0;
            ret = 0;
        }
	}

    LogJseGlueDebug("str<%s , %s>\n", ioStr, wrStr);
	return -1;
}

int 
IUBankDeviceCell::ioctl(const char* json, char* jsStr)
{
    int ret = -1;
    HString cmdJson = json;

    jsStr[0] = '\0';
    if (m_device) {
		const char *pStr = m_device->ioctl(cmdJson);
        if (pStr) {
            strncpy(jsStr, pStr, IJSEBUFFSIZE);
            jsStr[IJSEBUFFSIZE] = 0;
            ret = 0;
        }
    }

    LogJseGlueDebug("str<%s , %s>\n", json, jsStr);
    return ret;

}

bool 
IUBankDeviceCell::deleteEvent(int eventId)
{
    LogJseGlueDebug("value=%d\n", eventId);

	if (!m_device)
		return false;
    return m_device->deleteEvent(eventId);
}

bool 
IUBankDeviceCell::addEvent(int eventId)
{
    LogJseGlueDebug("value=%d\n", eventId);

	if (!m_device)
		return false;
	return m_device->addEvent(eventId);
}

//export
IUBankDevice* createIUBankDevice(const char* name)
{
    LogJseGlueDebug("name<%s>\n", name);
    return new IUBankDeviceCell;
}

