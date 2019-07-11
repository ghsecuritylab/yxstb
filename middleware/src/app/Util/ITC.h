
/* Inter-thread call */

#ifndef _ITC_H_
#define _ITC_H_

#include "MessageHandler.h"

#ifdef __cplusplus

#include <string>

namespace Hippo {

struct Result {
    enum Type {
    	Int,
    	Float,
    	Double,
    	String
    };
    Type type;
    union Value {
        int iValue;
        float fValue;
        double dValue;
        std::string *sValue;
    } u;
};

class ITC : public MessageHandler {
public:
    ITC(Result *syncChannel);
    ~ITC();

    int call(int what);
    int call(int what, Object *obj);
    int call(int what, int arg1, int arg2);
    int call(int what, int arg1, int arg2, Object *obj);

protected:
    virtual void handleMessage(Message *msg);
    void wakeUp();

private:
    Result *mSyncChannel;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _ITC_H_
