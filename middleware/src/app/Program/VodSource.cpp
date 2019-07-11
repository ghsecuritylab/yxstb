
#include "VodSource.h"


namespace Hippo {

VodSource::VodSource()
{
    m_url.clear();
    m_mediaCode.clear();
    m_entryID.clear();
    m_startTime.clear();
    m_endTime.clear();
    m_mediaType = 0;
    m_audioType = 0;
    m_videoType = 0;
    m_streamType = 0;
    m_drmType = 0;
    m_fingerPrint = 0;
    m_copyProtection = 0;
    m_allowTrickmode = 0;
    m_insertTime = 0;
    m_isPostive = 0;
    m_bandwidth = 0;
}

VodSource::~VodSource()
{
}

} // namespace Hippo
