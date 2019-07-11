#ifndef _LayerMixerDeviceZebra_H_
#define _LayerMixerDeviceZebra_H_

#include "LayerMixerDevice.h"

#ifdef __cplusplus

namespace Hippo {

class Rect;
class Canvas;
class Widget;
class WidgetSource;

class LayerMixerDeviceZebra : public LayerMixerDevice {
public:
    LayerMixerDeviceZebra();
    ~LayerMixerDeviceZebra();

    virtual void refresh(bool force = false);
};

} // namespace Hippo

#endif // __cplusplus

#endif // _LayerMixerDeviceZebra_H_
