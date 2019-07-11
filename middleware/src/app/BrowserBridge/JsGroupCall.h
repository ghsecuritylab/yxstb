#ifndef _JsGroupCall_H_
#define _JsGroupCall_H_

#include "JsCall.h"

#ifdef __cplusplus

namespace Hippo {

class JsGroupCall : public JsCall {
public:
    JsGroupCall();
    ~JsGroupCall();

    virtual int call(const char *subsections, int set, char *buffer, int length);
private:
    std::map<std::string, JsCallFunction> mFunctionMap;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _JsGroupCall_H_
