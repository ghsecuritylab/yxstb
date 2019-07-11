#ifndef JseHWPVR_h
#define JseHWPVR_h

#include "JseGroupCall.h"

#ifdef __cplusplus

class JseHWPVR : public JseGroupCall {
public:
	virtual int call(const char *name, const char *param, char *value, int length, int set);
    JseHWPVR();
    ~JseHWPVR();
};
//namespace Hippo {
int JseHWPVRInit();
//}
#endif

#endif // JseHWPVR_h

