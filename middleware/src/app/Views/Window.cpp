
#include "Window.h"

#include "Canvas.h"

namespace Hippo {

Window::Window()
{
    fWaitingOnInval = false;
}

Window::~Window()
{
}

void Window::resize(int pWidth, int pHeight)
{
}

bool Window::handleInval(const Rect& r)
{
	fDirtyRgn.op(r, Region::kUnion_Op);

	this->onHandleInval(r);

	return true;
}

bool Window::update(Rect* updateArea, Canvas* canvas)
{
    VIEW_LOG("debug here!\n");
    if (!fDirtyRgn.isEmpty()) {
        AutoCanvasRestore acr(canvas, true);

        canvas->clipRegion(fDirtyRgn);
        if (updateArea)
            *updateArea = fDirtyRgn.getBounds();

        // empty this now, so we can correctly record any inval calls that
        // might be made during the draw call.
        fDirtyRgn.setEmpty();

        this->draw(canvas);
        return true;
    }
	return false;
}

//////////////////////////////////////////////////////////////////////

void Window::onHandleInval(const Rect&)
{
}

} // namespace Hippo
