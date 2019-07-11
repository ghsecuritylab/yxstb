#ifndef _CPVRJSCALL_H_
#define _CPVRJSCALL_H_

#ifdef __cplusplus

#include "JsGroupCall.h"

namespace Hippo {

class NetworkJsCall : public JsGroupCall {
};

} // namespace Hippo




extern "C" {
#endif // __cplusplus

void cpvr_module_init();
void cpvr_module_enable();
void cpvr_module_disable();
void cpvrTaskResumeAfterReboot(int flag);

int cpvrListRefresh();
int getPvrListVersion(char*);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif

