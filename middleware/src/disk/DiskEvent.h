#ifndef DiskEvent_H
#define DiskEvent_H

#include <string>

namespace Hippo {

class DiskEvent{
public:
    DiskEvent(const std::string &event);
    DiskEvent();
    ~DiskEvent(){}
    int build(const char*, const char*, int pvrFlag);
    int report();
    void doUsbEvent();

private:
    std::string m_event;
};//DiskEvent

};//namespace Hippo

#endif //DiskEvent_H

