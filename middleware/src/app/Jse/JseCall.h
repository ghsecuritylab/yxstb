#ifndef _JseCall_H_
#define _JseCall_H_

//#include "SubsectionCall.h"

#ifdef __cplusplus

#include <string>


class JseCall /*: public SubsectionCall*/ {
public:
    JseCall(const char* name);
    ~JseCall();

    const char* name() { return m_name.c_str(); }
    virtual int call(const char *name, const char *param, char *value, int length, int set) = 0;
    virtual JseCall* getNode(const char* name, JseCall* fatherNode = NULL) = 0;

protected:
    std::string m_name;
};

#endif // __cplusplus

#endif // _JseCall_H_
