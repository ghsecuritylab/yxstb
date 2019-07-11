#ifndef _LayerMixerDevice_H_
#define _LayerMixerDevice_H_

#ifdef __cplusplus

namespace Hippo {

class Rect;
class Canvas;
class Widget;
class WidgetSource;

class LayerMixerDevice {
public:
    class Layer {
    public:
        enum Standard {
            S480 = 0,
            S576,
            S720,
            S1080
        };
        struct StandardSize {
            int width;
            int height;
        };

        static Standard closeStandard(int width, int height);
        static StandardSize &standardSize(Standard pStandard);

        static void convertToStandard(Standard pStandard, Rect* rect, WidgetSource* source);

        Layer();

        virtual Standard standard() = 0;
        virtual void setStandard(Standard) = 0;
        virtual void setCanvasMaxSize(int width, int height) = 0;
        virtual void setCanvasValidSize(int width, int height, bool ForcedToRefresh) = 0;
        virtual void calculateCanvasOffset() = 0;
        virtual bool repaint(Canvas* canvas = 0, bool copy = 0) = 0;
        virtual bool dirty() = 0;

        virtual Widget* attachChildToFront(Widget* child) = 0;
        virtual Widget* attachChildToBack(Widget* child) = 0;
        virtual bool hasVisibleChild() = 0;

        virtual void show();
        virtual void hide();

        int mCanvasMaxWidth, mCanvasMaxHeight;
        int mCanvasValidWidth, mCanvasValidHeight;
        int mCanvasOffsetX, mCanvasOffsetY;

        int mPlatformLayer;
        int platformLayer() { return mPlatformLayer; }
        bool mSoftLayer;
    };

    LayerMixerDevice();
    virtual ~LayerMixerDevice();

    void setContentOffset(float hOffset, float vOffset);
    void setContentScaler(float hScaler, float vScaler);

    void setStandard(Layer::Standard);

    int convertToDeviceCoordinates(Layer::StandardSize pSize, int pOffsetX, int pOffsetY, int &x, int &y, int &width, int &height);

    virtual void refresh(bool force = false);
    void display();

    Layer *topLayer() { return mTopLayer; }
    Layer *middleLayer() { return mMiddleLayer; }
    Layer *bottomLayer() { return mBottomLayer; }

    void setTopLayer(Layer *layer) { mTopLayer = layer; }
    void setMiddleLayer(Layer *layer) { mMiddleLayer = layer; }
    void setBottomLayer(Layer *layer) { mBottomLayer = layer; }

    Canvas* mCanvas;

protected:
    Layer *mTopLayer;
    Layer *mMiddleLayer;
    Layer *mBottomLayer;

    float mHOffset, mVOffset;
    float mHScaler, mVScaler;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _LayerMixerDevice_H_
