#ifndef JseHWNetDiagnoseTool_h
#define JseHWNetDiagnoseTool_h

#include "JseGroupCall.h"

#ifdef __cplusplus

int JseHWNetDiagnoseToolInit();


class JseHWNetDiagnoseTool : public JseGroupCall {
public:
    JseHWNetDiagnoseTool();
    ~JseHWNetDiagnoseTool();
};

#endif

#endif // JseHWNetDiagnoseTool_h

