#ifndef _Window_H_
#define _Window_H_

#include "View.h"
#include "Region.h"

#ifdef __cplusplus

namespace Hippo {

class Window : public View {
public:
            Window();
    virtual ~Window();

    void    resize(int pWidth, int pHeight);

    bool    isDirty() /*const*/ { return !fDirtyRgn.isEmpty(); }
    bool    update(Rect* updateArea, Canvas* canvas);

protected:
    // called if part of our bitmap is invalidated
    virtual void onHandleInval(const Rect&);

    // overrides from View
    virtual bool handleInval(const Rect&);

//private:
    //SkBitmap::Config    fConfig;
    //SkBitmap    fBitmap;
    Region    fDirtyRgn;

    bool fWaitingOnInval;

    typedef View INHERITED;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _Window_H_
