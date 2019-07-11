
#include "LayerMixerDevice.h"
#include "BrowserAgentTakin.h"
#include "Widget.h"
#include "Assertions.h"


#include "stdio.h"
#include "libzebra.h"

extern "C" {
#include "TAKIN_browser.h"
#include "TAKIN_setting_type.h"
}

#if defined(hi3560e)
extern "C" yresult ygp_HWLayer_SetPos(int layerHandle, int x, int y);
#endif

namespace Hippo {
static int sTopMagin = 0, sLeftMagin = 0;

struct Scaler {
    float xScaler;
    float yScaler;
};

static Scaler gScalers[4][4] = {
{{720.0/ 720.0, 480.0/ 480.0}, {720.0/ 720.0, 576.0/ 480.0}, {1280.0/ 720.0, 720.0/ 480.0}, {1920.0/ 720.0, 1080.0/ 480.0}},
{{720.0/ 720.0, 480.0/ 576.0}, {720.0/ 720.0, 576.0/ 576.0}, {1280.0/ 720.0, 720.0/ 576.0}, {1920.0/ 720.0, 1080.0/ 576.0}},
{{720.0/1280.0, 480.0/ 720.0}, {720.0/1280.0, 576.0/ 720.0}, {1280.0/1280.0, 720.0/ 720.0}, {1920.0/1280.0, 1080.0/ 720.0}},
{{720.0/1920.0, 480.0/1080.0}, {720.0/1920.0, 576.0/1080.0}, {1280.0/1920.0, 720.0/1080.0}, {1920.0/1920.0, 1080.0/1080.0}}
};

static LayerMixerDevice::Layer::StandardSize gStandardSizes[4] = {
{ 720,  480},
{ 720,  576},
{1280,  720},
{1920, 1080}
};

static LayerMixerDevice::Layer::Standard getCloseStandard(int width, int height)
{
    if (width > gStandardSizes[2].width || height > gStandardSizes[2].height) {
        return LayerMixerDevice::Layer::S1080;
    }
    else if (width > gStandardSizes[1].width || height > gStandardSizes[1].height) {
        return LayerMixerDevice::Layer::S720;
    }
    else if (width > gStandardSizes[0].width || height > gStandardSizes[0].height) {
        return LayerMixerDevice::Layer::S576;
    }
    else {
        //PAL
        return LayerMixerDevice::Layer::S576;
        //NTSC
        return LayerMixerDevice::Layer::S480;
    }
}

LayerMixerDevice::Layer::Standard
LayerMixerDevice::Layer::closeStandard(int width, int height)
{
    return getCloseStandard(width, height);
}

LayerMixerDevice::Layer::StandardSize &
LayerMixerDevice::Layer::standardSize(Standard pStandard)
{
    return gStandardSizes[pStandard];
}

void
LayerMixerDevice::Layer::convertToStandard(Standard pStandard, Rect* rect, WidgetSource* source)
{
    Scaler *scaler = &gScalers[source->standard][pStandard];
    rect->fLeft   = (int32_t)(source->x * scaler->xScaler);
    rect->fTop    = (int32_t)(source->y * scaler->yScaler);
    rect->fRight  = (int32_t)(source->w * scaler->xScaler) + rect->fLeft;
    rect->fBottom = (int32_t)(source->h * scaler->yScaler) + rect->fTop;
}

LayerMixerDevice::Layer::Layer()
	: mCanvasMaxWidth(0)
	, mCanvasMaxHeight(0)
	, mCanvasValidWidth(0)
	, mCanvasValidHeight(0)
	, mCanvasOffsetX(0)
	, mCanvasOffsetY(0)
	, mPlatformLayer(0)
	, mSoftLayer(false)
{
}

void
LayerMixerDevice::Layer::show()
{
    ygp_layer_setShow(mPlatformLayer, 1);
    ygp_layer_updateScreen();
}

void
LayerMixerDevice::Layer::hide()
{
    ygp_layer_setShow(mPlatformLayer, 0);
    ygp_layer_updateScreen();
}


LayerMixerDevice::LayerMixerDevice()
	: mTopLayer(NULL)
	, mMiddleLayer(NULL)
	, mBottomLayer(NULL)
	, mHOffset(0.0)
	, mVOffset(0.0)
	, mHScaler(1.0)
	, mVScaler(1.0)
{

}

LayerMixerDevice::~LayerMixerDevice()
{
}

void
LayerMixerDevice::setContentOffset(float hOffset, float vOffset)
{
    mHOffset = hOffset;
    mVOffset = vOffset;
}

void
LayerMixerDevice::setContentScaler(float hScaler, float vScaler)
{
    mHScaler = hScaler;
    mVScaler = vScaler;
}

void
LayerMixerDevice::setStandard(Layer::Standard pStandard)
{
    if (mTopLayer)
        mTopLayer->setStandard(pStandard);
    if (mMiddleLayer)
        mMiddleLayer->setStandard(pStandard);
    if (mBottomLayer)
        mBottomLayer->setStandard(pStandard);

#if !defined(GRAPHIC_SINGLE_LAYER)
    Layer::StandardSize &size = Layer::standardSize(pStandard);

    int screenWidth = 0, screenHeight = 0;

    ygp_layer_getScreenSize(&screenWidth, &screenHeight);
    LogRunOperDebug("The Screen width(%d) height(%d) LeftMagin(%d) TopMagin(%d) ratio(%f)\n", screenWidth, screenHeight, sLeftMagin, sTopMagin, (float)((float)(screenWidth - sLeftMagin * 2)/(float)screenWidth));

    float xScaler, yScaler;
    xScaler = (float)screenWidth / (float)size.width;
    yScaler = (float)screenHeight / (float)size.height;

    int targetX, targetY, targetW, targetH;

    if (mTopLayer != NULL && mTopLayer->mPlatformLayer != mMiddleLayer->mPlatformLayer) {
        if (mTopLayer->mCanvasValidWidth < size.width || mTopLayer->mCanvasValidHeight < size.height) {
            ygp_layer_setClipPosition(mTopLayer->mPlatformLayer, 0, 0, mTopLayer->mCanvasValidWidth, mTopLayer->mCanvasValidHeight);
            targetX = mTopLayer->mCanvasOffsetX * xScaler;
            targetY = mTopLayer->mCanvasOffsetY * yScaler;
            targetW = mTopLayer->mCanvasValidWidth * xScaler;
            targetH = mTopLayer->mCanvasValidHeight * yScaler;
            ygp_layer_setDisplayPosition(mTopLayer->mPlatformLayer, targetX, targetY, targetW, targetH);
        }
        else {
            ygp_layer_setClipPosition(mTopLayer->mPlatformLayer, 0, 0, size.width, size.height);
            ygp_layer_setDisplayPosition(mTopLayer->mPlatformLayer, 0, 0, screenWidth, screenHeight);
        }
    }

    if (mMiddleLayer->mSoftLayer) {
        char tBrowserOffset[32] = {0};

        if (mMiddleLayer->mCanvasValidWidth < size.width || mMiddleLayer->mCanvasValidHeight < size.height) {
            ygp_layer_setClipPosition(mMiddleLayer->mPlatformLayer, 0, 0, mMiddleLayer->mCanvasValidWidth, mMiddleLayer->mCanvasValidHeight);

            if(0 >= sLeftMagin && 0 >= sTopMagin) {
                targetX = mMiddleLayer->mCanvasOffsetX * xScaler;
                targetY = mMiddleLayer->mCanvasOffsetY * yScaler;
                targetW = mMiddleLayer->mCanvasValidWidth * xScaler;
                targetH = mMiddleLayer->mCanvasValidHeight * yScaler;
                ygp_layer_setDisplayPosition(mMiddleLayer->mPlatformLayer, targetX, targetY, targetW, targetH);
            }
            else {
                targetX = 0;
                targetY = 0;
                targetW = 0;
                targetH = 0;
                ygp_layer_setDisplayPosition(mMiddleLayer->mPlatformLayer,
                                             sLeftMagin,
                                             sTopMagin,
                                             screenWidth - sLeftMagin * 2,
                                             screenHeight - sTopMagin * 2);
            }

            snprintf(tBrowserOffset, 32, "%d %d %d %d", targetX, targetY, targetW, targetH);
            epgBrowserAgent().setTakinSettings(TAKIN_BROWSER_OFFSET_RECT, tBrowserOffset, strlen(tBrowserOffset));
        }
        else {
            ygp_layer_setClipPosition(mMiddleLayer->mPlatformLayer, 0, 0, size.width, size.height);

            ygp_layer_setDisplayPosition(mMiddleLayer->mPlatformLayer,
                                         sLeftMagin,
                                         sTopMagin,
                                         screenWidth - sLeftMagin * 2,
                                         screenHeight - sTopMagin * 2);

            snprintf(tBrowserOffset, 32, "0 0 0 0");
            epgBrowserAgent().setTakinSettings(TAKIN_BROWSER_OFFSET_RECT, tBrowserOffset, 32);
        }
    }
#if defined(hi3560e)
    else {
        if (mMiddleLayer->mCanvasValidWidth < size.width || mMiddleLayer->mCanvasValidHeight < size.height) {
            targetX = mMiddleLayer->mCanvasOffsetX * xScaler;
            targetY = mMiddleLayer->mCanvasOffsetY * yScaler;
            targetW = mMiddleLayer->mCanvasValidWidth * xScaler;
            targetH = mMiddleLayer->mCanvasValidHeight * yScaler;
#if defined(Jiangsu) && defined(EC1308H)
            ygp_HWLayer_SetPos( mMiddleLayer->mPlatformLayer, 40, 23 );
#else
            ygp_HWLayer_SetPos( mMiddleLayer->mPlatformLayer, targetX, targetY );
#endif
            char tBrowserOffset[32] = {0};
            sprintf(tBrowserOffset, "%d %d %d %d", targetX, targetY, targetW, targetH);
            // printf("Browser offset rect x(%d) y(%d) width(%d) height(%d)\n", targetX, targetY, targetW, targetH);
            epgBrowserAgent().setTakinSettings(TAKIN_BROWSER_OFFSET_RECT, tBrowserOffset, strlen(tBrowserOffset));
        }
    }
#endif
    if (mBottomLayer != NULL && mBottomLayer->mPlatformLayer != mMiddleLayer->mPlatformLayer) {
	    if (mBottomLayer->mCanvasValidWidth < size.width || mBottomLayer->mCanvasValidHeight < size.height) {
	        ygp_layer_setClipPosition(mBottomLayer->mPlatformLayer, 0, 0, mBottomLayer->mCanvasValidWidth, mBottomLayer->mCanvasValidHeight);
	        targetX = mBottomLayer->mCanvasOffsetX * xScaler;
	        targetY = mBottomLayer->mCanvasOffsetY * yScaler;
	        targetW = mBottomLayer->mCanvasValidWidth * xScaler;
	        targetH = mBottomLayer->mCanvasValidHeight * yScaler;
	        ygp_layer_setDisplayPosition(mBottomLayer->mPlatformLayer, targetX, targetY, targetW, targetH);
	    }
	    else {
	        ygp_layer_setClipPosition(mBottomLayer->mPlatformLayer, 0, 0, size.width, size.height);
	        ygp_layer_setDisplayPosition(mBottomLayer->mPlatformLayer, 0, 0, screenWidth, screenHeight);
    	}
    }
#endif
}

int
LayerMixerDevice::convertToDeviceCoordinates(Layer::StandardSize pSize, int pOffsetX, int pOffsetY, int &x, int &y, int &width, int &height)
{
    int screenWidth = 0, screenHeight = 0;

    ygp_layer_getScreenSize(&screenWidth, &screenHeight);

    float xScaler, yScaler;
    xScaler = (float)screenWidth / (float)pSize.width;
    yScaler = (float)screenHeight / (float)pSize.height;

    x = (int)((x + pOffsetX) * xScaler);
    y = (int)((y + pOffsetY) * yScaler);
    width = (int)(width * xScaler);
    height = (int)(height * yScaler);
    return 0;
}

void
LayerMixerDevice::refresh(bool force)
{
    bool repainted = force;

    if(mTopLayer)
        repainted |= mTopLayer->repaint();
    if(mMiddleLayer)
        repainted |= mMiddleLayer->repaint();
    if(mBottomLayer)
        repainted |= mBottomLayer->repaint();

    if (repainted) {
        ygp_layer_updateScreen();
    }
}

void
LayerMixerDevice::display()
{
    bool repainted = false;

    if(mTopLayer)
        repainted |= mTopLayer->repaint();
    if(mBottomLayer)
        repainted |= mBottomLayer->repaint();

    if (repainted) {
        ygp_layer_updateScreen();
    }
#if defined(hi3560e)
    if (!mMiddleLayer->mSoftLayer) {
        ygp_HWLayer_Update(mMiddleLayer->mPlatformLayer);
    }
#endif
}

} // namespace Hippo

/* 一下代码为垃圾 */
#include "SystemManager.h"

extern "C" void
LayerMixerDeviceSetLeftVertices(int leftmagin, int topmagin)
{
    extern void TakinSetFrameSizeUpdate(int frameSizeUpdate);

    if(0 <= leftmagin && Hippo::sLeftMagin != leftmagin) {
        Hippo::sLeftMagin = leftmagin;
        TakinSetFrameSizeUpdate(1);
    }
    if(0 <= topmagin && Hippo::sTopMagin != topmagin) {
        Hippo::sTopMagin = topmagin;
        TakinSetFrameSizeUpdate(1);
    }
    return;
}

extern "C" int
layerMixerDevice_middleLayerHandle(void)
{
	return Hippo::systemManager().mixer().middleLayer()->platformLayer();
}

extern "C" void
mid_plane_browser_positionOffset(int x, int y, int w, int h, int *my_x, int *my_y, int *my_w, int *my_h)
{
    int tFrameUpdate = 0, tFrameWidth = 0, tFrameHeight = 0;
    int tScreenWidth = 0, tScreenHeight = 0;
    int tCanvasOffsetX = 0, tCanvasOffsetY = 0;
    Hippo::LayerMixerDevice::Layer::StandardSize size;

    extern void TAKIN_get_frame_size(int *pFrameUpdate, int *pFrameWidth, int *pFrameHeight);

    TAKIN_get_frame_size(&tFrameUpdate, &tFrameWidth, &tFrameHeight);
    ygp_layer_getScreenSize(&tScreenWidth, &tScreenHeight);

    if(tFrameUpdate) {
        size = Hippo::LayerMixerDevice::Layer::standardSize(Hippo::LayerMixerDevice::Layer::closeStandard(tFrameWidth, tFrameHeight));

        if (tFrameWidth >= size.width)
            tCanvasOffsetX = 0;
        else
            tCanvasOffsetX = (size.width - tFrameWidth) >> 1;

        if (tFrameHeight >= size.height)
            tCanvasOffsetY = 0;
        else
            tCanvasOffsetY = (size.height - tFrameHeight) >> 1;

    } else {
        size = Hippo::LayerMixerDevice::Layer::standardSize(Hippo::systemManager().mixer().middleLayer()->standard());
        tCanvasOffsetX = Hippo::systemManager().mixer().middleLayer()->mCanvasOffsetX;
        tCanvasOffsetY = Hippo::systemManager().mixer().middleLayer()->mCanvasOffsetY;
    }

#if defined(ANDROID)
    if (tFrameWidth < size.width || tFrameHeight < size.height) {
        x = x * tScreenWidth / tFrameWidth;
        y = y * tScreenHeight / tFrameHeight;
        w = w * tScreenWidth / tFrameWidth;
        h = h * tScreenHeight / tFrameHeight;
    }
    else
        Hippo::systemManager().mixer().convertToDeviceCoordinates(size, tCanvasOffsetX, tCanvasOffsetY, x, y, w, h);
#else
    if((0 < Hippo::sTopMagin || 0 < Hippo::sLeftMagin) && (tFrameWidth < size.width || tFrameHeight < size.height)) {
        tCanvasOffsetX = 0;
        tCanvasOffsetY = 0;
        x = x * size.width / tFrameWidth;
        y = y * size.height / tFrameHeight;
        w = w * size.width / tFrameWidth;
        h = h * size.height / tFrameHeight;
    }

    Hippo::systemManager().mixer().convertToDeviceCoordinates(size, tCanvasOffsetX, tCanvasOffsetY, x, y, w, h);
#endif

    if(0 < Hippo::sTopMagin || 0 < Hippo::sLeftMagin) {
        float WidthRatio = 0.0, HeightRatio = 0.0;

        WidthRatio = (float)((float)(tScreenWidth - Hippo::sLeftMagin * 2) / (float)tScreenWidth);
        HeightRatio = (float)((float)(tScreenHeight - Hippo::sTopMagin * 2) / (float)tScreenHeight);
        LogRunOperDebug("The full screen ratio(%f)\n", WidthRatio);

        x = x * WidthRatio + Hippo::sLeftMagin;
        y = y * HeightRatio + Hippo::sTopMagin;
        w = w * WidthRatio;
        h = h * HeightRatio;
        LogRunOperDebug("The video magin x(%d)y(%d)w(%d)h(%d)\n", x, y, w, h);
    }

    if (my_x) {
        *my_x = x;
    }
    if (my_y) {
        *my_y = y;
    }
    if (my_w) {
        *my_w = w;
    }
    if (my_h) {
        *my_h = h;
    }
}
