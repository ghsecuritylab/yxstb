#ifndef AdvertisementList_h
#define AdvertisementList_h

#include "ProgramVOD.h"

#define AD_LIST_MAX_LEN 100
#ifdef __cplusplus

class AdvertisementList {
public:
    AdvertisementList();
    ~AdvertisementList();

	void advertisingSpots(Hippo::ProgramVOD*);
    void init(int);
    void add(const char*);
    void reset(void);

private:
    std::string *m_adList[AD_LIST_MAX_LEN];
    int m_adIndex;
    int m_adListLen;
};

AdvertisementList &advertisementList();

#endif // __cplusplus

#endif // AdvertisementList_h
