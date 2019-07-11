
#include "StandardScreen.h"

#include "Widget.h"
#include "Canvas.h"

#include "Message.h"
#include "MessageTypes.h"
#include "NativeHandler.h"

extern "C" int ygp_layer_updateScreen(void);

namespace Hippo {

StandardScreen::StandardScreen(Standard pStandard, Canvas* canvas)
	: mCurrentStandard(pStandard)
	, mCanvas(canvas)
{
    StandardSize &size = standardSize(pStandard);

    setLoc(0, 0);
    setSize(size.width, size.height);
}

StandardScreen::~StandardScreen()
{
}

void
StandardScreen::setStandard(Standard pStandard)
{
    StandardSize &size = standardSize(pStandard);
    setSize(size.width, size.height);
}

void
StandardScreen::setCanvasMaxSize(int pWidth, int pHeight)
{
    mCanvasMaxWidth = pWidth;
    mCanvasMaxHeight = pHeight;
}

void
StandardScreen::setCanvasValidSize(int pWidth, int pHeight, bool ForcedToRefresh)
{
    int oldValidWidth = mCanvasValidWidth;
    int oldValidHeight = mCanvasValidHeight;

    if (pWidth > mCanvasMaxWidth)
        mCanvasValidWidth = mCanvasMaxWidth;
    else
        mCanvasValidWidth = pWidth;

    if (pHeight > mCanvasMaxHeight)
        mCanvasValidHeight = mCanvasMaxHeight;
    else
        mCanvasValidHeight = pHeight;

    if ((oldValidWidth != mCanvasValidWidth) || (oldValidHeight != mCanvasValidHeight) || ForcedToRefresh) {
        inval(NULL);
    }
}

void
StandardScreen::calculateCanvasOffset()
{
    //printf("mCanvasValidWidth(%d)(%d) mCanvasValidHeight(%d)(%d)\n", mCanvasValidWidth, width(), mCanvasValidHeight, height());
    if (mCanvasValidWidth >= width())
        mCanvasOffsetX = 0;
    else
        mCanvasOffsetX = (width() - mCanvasValidWidth) >> 1;

    if (mCanvasValidHeight >= height())
        mCanvasOffsetY = 0;
    else
        mCanvasOffsetY = (height() - mCanvasValidHeight) >> 1;
}

bool
StandardScreen::repaint(Canvas* canvas, bool copy)
{
    return update(NULL, mCanvas);
}

bool
StandardScreen::dirty()
{
    return INHERITED::isDirty();
}

Widget*
StandardScreen::attachChildToFront(Widget* child)
{
    if (child == NULL)
        return NULL;

    WidgetSource *source = child->getSource();

    Rect rect;
    convertToStandard(mCurrentStandard, &rect, source);
    child->setLoc(rect.fLeft, rect.fTop);
    child->setSize(rect.width(), rect.height());

    INHERITED::attachChildToFront(child);
    return child;
}

Widget*
StandardScreen::attachChildToBack(Widget* child)
{
    if (child == NULL)
        return NULL;

    WidgetSource *source = child->getSource();

    Rect rect;
    convertToStandard(mCurrentStandard, &rect, source);
    child->setLoc(rect.fLeft, rect.fTop);
    child->setSize(rect.width(), rect.height());

    INHERITED::attachChildToBack(child);
    return child;
}

bool
StandardScreen::hasVisibleChild()
{
    B2FIter iter(this);
    View* child;

    while ((child = iter.next()) != NULL) {
        if (child->isVisible())
            return true;
    }
    return false;
}

void
StandardScreen::onDraw(Canvas* canvas)
{
    cairo_t* cr = canvas->mCairo;

    cairo_save(cr);
    cairo_rectangle(cr, 0, 0, width(), height());
    cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
    cairo_fill(cr);
    cairo_restore(cr);
}

void
StandardScreen::onSizeChange()
{
    mCurrentStandard = closeStandard(width(), height());
    calculateCanvasOffset();

    B2FIter iter(this);
    View* child;

    while ((child = iter.next()) != NULL) {
        Rect rect;
        convertToStandard(mCurrentStandard, &rect, ((Widget*)child)->getSource());
        child->setLoc(rect.fLeft, rect.fTop);
        child->setSize(rect.width(), rect.height());
    }
}

void
StandardScreen::onHandleInval(const Rect&)
{
    sendMessageToNativeHandler(MessageType_Repaint, 0, 0, 0);
}

} // namespace Hippo
