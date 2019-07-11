#ifndef JseHWFile_h
#define JseHWFile_h

#include "JseGroupCall.h"

#ifdef __cplusplus

class JseHWFile : public JseGroupCall {
public:
    JseHWFile();
    ~JseHWFile();
};

class JseHWMediaInfo : public JseGroupCall {
public:
    JseHWMediaInfo();
    ~JseHWMediaInfo();
};

int JseHWFileInit();

#endif

#endif // JseHWFile_h

