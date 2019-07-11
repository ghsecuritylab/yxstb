#ifndef _BrowserViewTakin_H_
#define _BrowserViewTakin_H_

#include "BrowserView.h"

#ifdef __cplusplus

namespace Hippo {

class BrowserViewTakin : public BrowserView {
public:
    BrowserViewTakin(int platformLayer);
    ~BrowserViewTakin();

    void setContentSize(int pWidth, int pHeight);

protected:
    /** Override this to draw inside the view. Be sure to call the inherited version too */
    virtual void onDraw(Canvas*);
};

} // namespace Hippo

#endif // __cplusplus

#endif // _BrowserViewTakin_H_
