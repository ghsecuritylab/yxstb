
#include "StandardViewGroup.h"

#include "Widget.h"
#include "Canvas.h"
#include "StandardScreen.h"

#include "Message.h"
#include "MessageTypes.h"
#include "NativeHandler.h"

extern "C" int ygp_layer_updateScreen(void);

namespace Hippo {

StandardViewGroup::StandardViewGroup(Standard pStandard, StandardScreen *parent)
	: mCurrentStandard(pStandard)
	, mParent(parent)
{
    StandardSize &size = standardSize(pStandard);

    setLoc(0, 0);
    setSize(size.width, size.height);
}

StandardViewGroup::~StandardViewGroup()
{
}

void
StandardViewGroup::setStandard(Standard pStandard)
{
    StandardSize &size = standardSize(pStandard);

    setSize(size.width, size.height);

    if (size.width < mCanvasMaxWidth)
        mCanvasValidWidth = size.width;
    else
        mCanvasValidWidth = mCanvasMaxWidth;
    if (size.height < mCanvasMaxHeight)
        mCanvasValidHeight = size.height;
    else
        mCanvasValidHeight = mCanvasMaxHeight;

}

void
StandardViewGroup::setCanvasMaxSize(int pWidth, int pHeight)
{
    mCanvasMaxWidth = pWidth;
    mCanvasMaxHeight = pHeight;
}

void
StandardViewGroup::setCanvasValidSize(int pWidth, int pHeight, bool ForcedToRefresh)
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

    if ((oldValidWidth != mCanvasValidWidth) || (oldValidHeight != mCanvasValidHeight) || ForcedToRefresh)
        inval(NULL);

}

void
StandardViewGroup::calculateCanvasOffset()
{

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
StandardViewGroup::repaint(Canvas* canvas, bool copy)
{
    return mParent->repaint();
}

bool
StandardViewGroup::dirty()
{
    return false;
}

Widget*
StandardViewGroup::attachChildToFront(Widget* child)
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
StandardViewGroup::attachChildToBack(Widget* child)
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

void
StandardViewGroup::onSizeChange()
{
    mCurrentStandard = closeStandard(width(), height());
    calculateCanvasOffset();

    B2FIter iter(this);
    View*   child;

    while ((child = iter.next()) != NULL) {
        Rect rect;
        convertToStandard(mCurrentStandard, &rect, ((Widget*)child)->getSource());
        child->setLoc(rect.fLeft, rect.fTop);
        child->setSize(rect.width(), rect.height());
    }
}

} // namespace Hippo
