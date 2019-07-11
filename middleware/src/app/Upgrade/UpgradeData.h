#ifndef _UpgradeData_H_
#define _UpgradeData_H_

#ifdef __cplusplus

#include <string>

namespace Hippo {

class PlatBuffer;
class TempFile;

class UpgradeData {
public:
    enum Type {
    	UDT_TempBuffer = 0x01,
    	UDT_TempFile   = 0x02
    };

    UpgradeData(int pSize);
    UpgradeData(const char* filePath);
    ~UpgradeData();

    Type type() { return m_type; }
    PlatBuffer* tempBuffer() { return m_tempBuffer; }
    TempFile* tempFile() { return m_tempFile; }

    int open(char openType);
    void close();

    int read(void* buffer, int length);
    int write(void* data, int length);

    int seek(int offset);
    int size();
    void unlink();	

private:
    Type m_type;
    int m_bufferSize;
    PlatBuffer* m_tempBuffer;
    TempFile* m_tempFile;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _UpgradeData_H_
