
#include "SystemManager.h"
#include "LogoWidget.h"

#include "Canvas.h"

#include "libzebra.h"

namespace Hippo {

LogoWidget::LogoWidget(WidgetSource *source)
	: Widget(source)
	, mImage(NULL)
	, mImageLength(0)
{
}

LogoWidget::~LogoWidget()
{
}

void
LogoWidget::setLogo(unsigned char *pImage, int pImageLen)
{
    mImage = pImage;
    mImageLength = pImageLen;

#ifndef LOGO_STRETCH
    int tempWidth , tempHeight;
    LayerMixerDevice::Layer::StandardSize size;
    Rect rect;

    int_img_info(pImage, pImageLen, &tempWidth, &tempHeight);
#if defined(hi3560e)
    // 对于3560E的bootlogo图使用浏览器展示，但刚开机时用的是osd展示
    // 在boot页面的8%时会清除osd的展示，
    // 为了不产生黑屏，先让浏览器展示bootlogo，然后再清除osd展示
    // 的bootlogo，如果图片分辨率小与浏览器的，就会有osd展示
    // 的bootlogo遮挡8%进度条的现象，因此做此强制措施
    if (tempWidth < 640)
        tempWidth = 640;
    if (tempHeight < 530)
        tempHeight = 530;
#endif

    int tScreenWidth = 0, tScreenHeight = 0;
    ygp_layer_getScreenSize(&tScreenWidth, &tScreenHeight);
    mSource->standard = LayerMixerDevice::Layer::closeStandard(tScreenWidth, tScreenHeight);
    size = LayerMixerDevice::Layer::standardSize(mSource->standard);

    mSource->x= (size.width - tempWidth) / 2;
    mSource->y= (size.height - tempHeight) / 2;
    mSource->w = tempWidth;
    mSource->h = tempHeight;

    LayerMixerDevice::Layer::convertToStandard(Hippo::systemManager().mixer().bottomLayer()->standard(), &rect, mSource);
    setLoc(rect.fLeft, rect.fTop);
    setSize(rect.width(), rect.height());
#endif
    inval(NULL);
    return ;
}

unsigned char *
LogoWidget::getLogo(void)
{
    return mImage;
}

void
LogoWidget::onDraw(Canvas *canvas)
{
    Rect bounds;

    getLocalBounds(&bounds);

    mSource->image = mImage;
    mSource->imageLength = mImageLength;
    if (mSource->image){
        drawImage(canvas, &bounds, mSource->image, mSource->imageLength);
    }
    if (mSource->text){
        drawText(canvas, &bounds, mSource->text);
    }
}

} // namespace Hippo

