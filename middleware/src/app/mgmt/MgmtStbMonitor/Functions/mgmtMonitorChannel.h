#ifndef _mgmtMonitorChannel_H_
#define _mgmtMonitorChannel_H_


#ifdef __cplusplus
extern "C"{
#endif
namespace Hippo {
int mgmtGetChannelList_stor();
const char* mgmtGetChannelURL_byId(int id);
} // namespace Hippo

#ifdef __cplusplus
}
#endif

#endif // _mgmtMonitorChannel_H_
