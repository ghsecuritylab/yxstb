#ifndef JseHWRecordFiles_h
#define JseHWRecordFiles_h

#include "JseGroupCall.h"

#ifdef __cplusplus

class JseHWRecordFiles : public JseGroupCall {
public:
	virtual int call(const char *name, const char *param, char *value, int length, int set);
    JseHWRecordFiles();
    ~JseHWRecordFiles();
};

#endif

#endif // JseHWRecordFiles_h

