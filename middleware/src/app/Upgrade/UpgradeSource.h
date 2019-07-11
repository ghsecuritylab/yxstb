#ifndef _UpgradeSource_H_
#define _UpgradeSource_H_

#include "Object.h"
#include <stdint.h>
#include <string>

#ifdef __cplusplus

namespace Hippo {

class TempFile;
class TempBuffer;
class RingBuffer;
class DataSource;

class UpgradeSource : public Object {
public:
    UpgradeSource();
    ~UpgradeSource();

    enum BufferType {
        USBT_Unknow = 0x0,
    	USBT_TempFile = 0x01,
    	USBT_TempBuffer = 0x02,
    	USBT_RingBuffer = 0x04,
    	USBT_RingBufferFragment = 0x08
    };
    
    virtual int Type() = 0;
    virtual BufferType bufferType() { return m_bufferType; }

    virtual TempFile* getTempFile();
    virtual TempBuffer* getTempBuffer();
    virtual RingBuffer* getRingBuffer();
    virtual bool setSourceAddress(int);
    void setSuccessedContent();
    bool successedContent(uint32_t content) { return m_successedConent & content; }
    int currentContent() { return m_currentContent; }
    
    int m_version; // ´ýÉý¼¶µÄ°æ±¾ºÅ
    int m_logoVersion;
    int m_settingVersion;
    int m_provider; // 0: ctc 1: cu	
    bool m_force;
    bool m_prePrompt;
    bool m_postPrompt;
    bool m_ignoreVersion;
    std::string m_logomd5;
    std::string m_softwareSourceAddr;
    std::string m_logoSourceAddr;
    std::string m_settingSourceAddr;  
    
    int m_currentContent; // normal, logo, setting
    uint32_t m_successedConent;
              
    DataSource* m_dataSource;
    
protected:
    BufferType m_bufferType;
    union Buffer {
    	TempFile* m_file;
    	TempBuffer* m_tempBuffer;
    	RingBuffer* m_ringBuffer;
    } m_buffer;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _UpgradeSource_H_
