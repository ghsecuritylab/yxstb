#ifndef JseTools_h
#define JseTools_h

#include "JseGroupCall.h"

#ifdef __cplusplus

class JseKeyBoard : public JseGroupCall {
public:
    JseKeyBoard();
    ~JseKeyBoard();
};

int JseToolsInit();

#endif
#ifdef __cplusplus
extern "C" {
#endif

void appFactorySet(int flag);
#ifdef __cplusplus
}
#endif //__cplusplus

#endif // JseTools_h
