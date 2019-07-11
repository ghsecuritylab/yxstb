
#include "AudioVolumeWidget.h"

#include "Canvas.h"


namespace Hippo {

AudioVolumeWidget::AudioVolumeWidget(WidgetSource *source)
	: Widget(source)
	, mMaxValue(20)
	, mCurrentValue(0)
{
}

AudioVolumeWidget::~AudioVolumeWidget()
{
}

int 
AudioVolumeWidget::setValue(int value)
{
    int oldValue = mCurrentValue;
    mCurrentValue = value;
    inval(NULL);
    return oldValue;
}

int 
AudioVolumeWidget::getValue()
{
    return mCurrentValue;
}

void 
AudioVolumeWidget::onDraw(Canvas* canvas)
{
    double poleUnitWidth, poleWidth, poleHeight;
    int i;

    poleUnitWidth = (double)(mSource->w - mSource->h) / (double)mMaxValue;
    poleWidth = poleUnitWidth / 2;
    poleHeight = (double)mSource->h;

    cairo_t* cr = canvas->mCairo;

    cairo_save(cr);

    cairo_scale(cr, (double)width() / (double)(mSource->w), (double)height() / (double)(mSource->h));

    cairo_set_source_rgba(cr, 0x00, 0xff, 0x00, 0xff);
    for (i = 0; i < mCurrentValue; i++) {
        cairo_rectangle(cr, i * poleUnitWidth, 0, poleWidth, poleHeight);
        cairo_fill(cr);
    }

    cairo_set_source_rgba(cr, 0x80, 0x80, 0x80, 0x0f);
    for (i = mCurrentValue; i < mMaxValue; i++) {
        cairo_rectangle(cr, i * poleUnitWidth, 0, poleWidth, poleHeight);
        cairo_fill(cr);
    }

    Rect rect;
    rect.set(mSource->w - mSource->h, 0, mSource->w, mSource->h);
    cairo_set_source_rgba(cr, 0x00, 0xff, 0x00, 0xff);

    char buffer[8];
    sprintf(buffer, "%d", mCurrentValue);
    drawText(canvas, &rect, buffer);

    cairo_restore(cr);
}

} // namespace Hippo
