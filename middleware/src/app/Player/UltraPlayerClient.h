#ifndef _UltraPlayerClient_H_
#define _UltraPlayerClient_H_

#ifdef __cplusplus

namespace Hippo {

class UltraPlayerClient {
public:
    enum Type {
    	Browser,
    	Monitor
    };

    virtual Type type() = 0;
    virtual int instanceId() = 0;

    virtual void onDestroy() = 0;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _UltraPlayerClient_H_
