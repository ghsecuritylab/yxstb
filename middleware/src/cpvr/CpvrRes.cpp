#include "CpvrAssertions.h"
#include "CpvrRes.h"
#include "CpvrAuxTools.h"
#include "CpvrTaskManager.h"
#include "CpvrList.h"

namespace Hippo {

CpvrRes::CpvrRes()
{
}

CpvrRes::~CpvrRes()
{
}

int
CpvrRes::onAcquireResource()
{
    need_handle_queue_node_add(MSG_TASK_START_WITHOUT_REQ_RES, (char *)mScheduleID.c_str(), getLocalTime());
}

int
CpvrRes::onLoseResource()
{
    need_handle_queue_node_add(MSG_TASK_STOP, (char *)mScheduleID.c_str(), getLocalTime());
}

int
CpvrRes::closeForResource()
{
    need_handle_queue_node_add(MSG_TASK_CLOSE, (char *)mScheduleID.c_str(), getLocalTime());
}

} // namespace Hippo

