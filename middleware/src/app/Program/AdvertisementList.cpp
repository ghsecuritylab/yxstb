
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "AdvertisementList.h"
#include "ProgramParserC10.h"

#include "ind_mem.h"
#include "ProgramAssertions.h"

using namespace Hippo;

/*���岥�·��� :�·�����б�����channellist��ÿ���·��������ʱ����־λ
  ������岥��Ȼ�󱣴��·��Ĺ��URL���ṹ�壬ÿ�β���ǰ������־λ
  �Ƿ�Ϊ���岥����ǰ��������ܹ�����Ƿ�һ�£����3���жϾ�OK����
  ������岥������ֱ�Ӳ���Ƭ*/
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

