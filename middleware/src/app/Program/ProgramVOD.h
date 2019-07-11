#ifndef _ProgramVOD_H_
#define _ProgramVOD_H_

#include "Program.h"
#include "VodSource.h"

#include <vector>

#ifdef __cplusplus

namespace Hippo {

class ProgramVOD : public Program {
public:
    ProgramVOD();
    ~ProgramVOD();
     typedef enum {
    	TYPE_CHANNEL = 1,
    	TYPE_VOD,
    	TYPE_TVOD,
    	TYPE_MUSIC,
    	TYPE_PVR = 10,
    	TYPE_DOWNLOAD,
    	TYPE_HDPlayer,
    	TYPE_DLNA,
    	TYPE_DVB_CHANNEL,
    	TYPE_STBMONITOR
    } MEDIATYPE;

    ProgramType getType() { return PT_VOD; }
    virtual int getNumberID();
    virtual const char *getStringID();
    virtual VodSource *getVodSource(int index);

    int addVODSource(VodSource *pNode);

    int getVodSourceCount();

protected:
    std::vector<VodSource*> m_advlist;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _ProgramInfo_H_
