#ifndef _StringData_H_
#define _StringData_H_

#include "Object.h"

#ifdef __cplusplus

namespace Hippo {

class StringData : public Object {
public:
    StringData(const char* data);
    ~StringData();
    const int getSize() const { return mSize; }
    const char* getData() const { return mData; }
private:
    char* mData;
    int mSize;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _StringData_H_
