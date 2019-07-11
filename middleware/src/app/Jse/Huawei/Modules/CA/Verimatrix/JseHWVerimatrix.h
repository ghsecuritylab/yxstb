#ifndef _JseHWVerimatrix_H_
#define _JseHWVerimatrix_H_

#include "JseGroupCall.h"

#ifdef __cplusplus

class JseHWVerimatrix : public JseGroupCall {
public:
    JseHWVerimatrix();
    ~JseHWVerimatrix();
    
    virtual int call(const char *name, const char *param, char *value, int length, int set);
};

#endif // __cplusplus

#endif // _JseHWVerimatrix_H_