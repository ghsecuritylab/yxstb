
#include "UpgradeAssertions.h"
#include "UpgradeData.h"

#include "TempFile.h"


namespace Hippo {

UpgradeData::UpgradeData(int pSize)
	: m_type(UDT_TempBuffer)
	, m_bufferSize(pSize)
	, m_tempBuffer(0)
	, m_tempFile(0)
{
}

UpgradeData::UpgradeData(const char* filePath)
	: m_type(UDT_TempFile)
	, m_bufferSize(0)
	, m_tempBuffer(0)
{
    m_tempFile = new TempFile(filePath);
}

UpgradeData::~UpgradeData()
{
    if (m_type == UDT_TempFile)
        delete m_tempFile;
}

int 
UpgradeData::open(char openType)
{
    if (m_type == UDT_TempBuffer)
        return -1;
    else
        return m_tempFile->open(openType);
}

void 
UpgradeData::close()
{
    if (m_type == UDT_TempBuffer)
        return;
    else
        return m_tempFile->close();
}

int 
UpgradeData::read(void* buffer, int length)
{
    if (m_type == UDT_TempBuffer)
        return -1;
    else
        return m_tempFile->read(buffer, length);
}

int 
UpgradeData::write(void* data, int length)
{
    if (m_type == UDT_TempBuffer)
        return -1;
    else
        return m_tempFile->write(data, length);
}

int 
UpgradeData::seek(int offset)
{
    if (m_type == UDT_TempBuffer)
        return -1;
    else
        return m_tempFile->seek(offset);
}

int 
UpgradeData::size()
{
    if (m_type == UDT_TempBuffer)
        return -1;
    else
        return m_tempFile->size();
}

void
UpgradeData::unlink()
{
    m_tempFile->unlink();
	
}

} // namespace Hippo
