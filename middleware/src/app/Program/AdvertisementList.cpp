
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "AdvertisementList.h"
#include "ProgramParserC10.h"

#include "ind_mem.h"
#include "ProgramAssertions.h"

using namespace Hippo;

/*广告插播新方案 :下发广告列表，类似channellist，每次下方广告总数时立标志位
  进入广告插播，然后保存下发的广告URL到结构体，每次播放前，检测标志位
  是否为广告插播，当前广告数和总广告数是否一致，如果3个判断均OK，则
  进入广告插播，否则直接播正片*/
AdvertisementList::AdvertisementList()
    :m_adIndex(0)
	,m_adListLen(0)
{
}

AdvertisementList::~AdvertisementList()
{
}

void
AdvertisementList::advertisingSpots(Hippo::ProgramVOD* m_vod)
{
    if (m_adListLen == 0)
        PROGRAM_LOG_ERROR("m_adListLen is %d\n", m_adListLen);

    if (m_adIndex != m_adListLen)
        PROGRAM_LOG_ERROR("adlist num is err,current = %d,totol = %d\n", m_adIndex, m_adListLen);

	if (m_adListLen != 0 && m_adIndex == m_adListLen) {	//change for debug

		int i;
		for (i = 0; i < m_adListLen; i ++) {
			m_vod->addVODSource(programParser().getVodSourceByStr(*m_adList[i]));
		}
		reset();//
	}
}

void
AdvertisementList::init(int adcount)
{
    if (adcount <= 0 || adcount >= AD_LIST_MAX_LEN) {
        PROGRAM_LOG_ERROR("adcount is %d\n", adcount);
		return;
    }

    if (m_adListLen != 0) {
        reset();
    }
    m_adListLen = adcount;
    m_adIndex = 0;
    int tIndex;
    for (tIndex = 0; tIndex < m_adListLen; tIndex++) {
        m_adList[tIndex] = new std::string("");
    }

}

void
AdvertisementList::add(const char* str)
{
    if (!str) {
        PROGRAM_LOG_ERROR("str is null\n");
		return;
	}

    if (!m_adListLen) {
        PROGRAM_LOG_ERROR("m_adListLen is %d\n", m_adListLen);
		return;
	}

	*m_adList[m_adIndex] = str;
    m_adIndex++;
}

void
AdvertisementList::reset(void)
{
    int tIndex;
    for (tIndex = 0; tIndex < m_adListLen; tIndex++) {
        delete m_adList[tIndex];
    }
    m_adIndex = 0;
    m_adListLen = 0;
}

static AdvertisementList g_advertisementList;

AdvertisementList &advertisementList()
{
    return g_advertisementList;
}

