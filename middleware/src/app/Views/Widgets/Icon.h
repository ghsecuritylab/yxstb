#ifndef _Icon_H_
#define _Icon_H_

#include "Widget.h"

#ifdef __cplusplus

namespace Hippo {

class Icon : public Widget {
public:
    Icon(WidgetSource *source);
    ~Icon();

protected:
    /** Override this to draw inside the view. Be sure to call the inherited version too */
    virtual void onDraw(Canvas*);
};

} // namespace Hippo

#endif // __cplusplus

#endif // _Icon_H_
