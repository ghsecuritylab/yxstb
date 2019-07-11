#ifndef _OSDLayer_H_
#define _OSDLayer_H_

#include "StandardScreen.h"

#ifdef __cplusplus

namespace Hippo {

class OSDLayer : public StandardScreen {
public:
    OSDLayer(Standard initStandard);
    ~OSDLayer();

    virtual bool repaint(Canvas* canvas = 0, bool copy = 0);

protected:
    /** Override this to draw inside the view. Be sure to call the inherited version too */
    virtual void onDraw(Canvas*);
};

} // namespace Hippo

#endif // __cplusplus

#endif // _OSDLayer_H_
