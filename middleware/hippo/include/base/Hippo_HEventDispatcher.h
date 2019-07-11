#ifndef __HIPPO_EVENTDISPATCHER_H__
#define __HIPPO_EVENTDISPATCHER_H__

#include <stack>
#include "Hippo_OS.h"

namespace Hippo{
class HEvent;
class HActiveObject ;
class HEventDispatcher {
public:
    typedef enum{
        EventStatus_eUnknown = 0,
        /* has been process, break up the event routing. */
        EventStatus_eProcessed,
        /* should continue routing. */
        EventStatus_eIgnored,
        EventStatus_eContinue,
    } event_status_e;
};
}
#endif
