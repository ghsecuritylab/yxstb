#ifndef _JsCall_H_
#define _JsCall_H_

#include <vector>
#include "SubsectionCall.h"

#ifdef __cplusplus

namespace Hippo {

typedef int (* JsCallFunction)(int set, char *buffer, int length);

class JsCall : public SubsectionCall {
public:
    JsCall();
    ~JsCall();

    virtual int call(const char *subsections, int set, char *buffer, int length);
private:
    std::vector<JsCall *> mGroups;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _JsCall_H_
