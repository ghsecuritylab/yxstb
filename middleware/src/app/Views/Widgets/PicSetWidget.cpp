/**
 * @file PicSetWidget.cpp
 * @brief Design gif animation
 * @author Michael
 * @version 1.0
 * @date 2012-07-27
 */
#include "PicSetWidget.h"
#include "Canvas.h"
#include "SystemManager.h"

namespace Hippo {

/**
 * @brief PicSetWidget construct
 *
 * @param nSource the attributes (standard, cordinate x, cordinate y, width, height and so on) of the widget 
 */

PicSetWidget::PicSetWidget(WidgetSource* nSource, int nMaxNumber)
    : Widget(nSource), mMaxNumber(nMaxNumber), mOrderNumber(0)
{
    if (nMaxNumber > 0) {
        mImagesAddr.resize(mMaxNumber + 1, 0);
        mImagesLen.resize(mMaxNumber + 1, 0);
    }
}

/**
 * @brief ~PicSetWidget - destruct
 */
PicSetWidget::~PicSetWidget()
{
    mImagesAddr.clear();
    mImagesLen.clear();
}

/**
 * @brief setOrderNumber use it to set the first gif picture NO.
 *
 * @param nOrderID the order number of gif 
 */
void 
PicSetWidget::setOrderNumber(int nOrderID) 
{ 
    if (0 <= nOrderID && nOrderID <= mMaxNumber)
        mOrderNumber = nOrderID; 
}

/**
 * @brief setSlideImage set each (total 16) images about clock gif resource.
 *
 * @param nOrderID the NO. of the images
 * @param nImageAddress the binary data address of the images
 * @param nImageLength the length of the images binary data.
 */
void 
PicSetWidget::insert(int nOrderID, void* nImageAddress, int nImageLength)
{
    if ( 0 <= nOrderID && nOrderID <= mMaxNumber && nImageAddress) {
        mImagesAddr[nOrderID] = nImageAddress;
        mImagesLen[nOrderID] = nImageLength;
    }
}

/**
 * @brief onDraw the virtual method inherited, will be called by execute 'setVisibleP' before set 'inval(NULL)'
 *      
 * @note the method is called asynchronously 
 *
 * @param nCanvas 
 */
void 
PicSetWidget::onDraw(Canvas* nCanvas)
{    
    Rect lBounds;
    getLocalBounds(&lBounds);
	
	if (mImagesAddr[mOrderNumber]) {
        mSource->image = mImagesAddr[mOrderNumber];
    }
    if (mSource->image) {
        VIEW_LOG("drawImage [%s] start\n", (unsigned char*)mSource->image);
		{		
			Image img((unsigned char*)mSource->image); 
			drawImage(nCanvas, &lBounds, &img); //draw	
		} 
        mOrderNumber = (mOrderNumber + 1) % (mMaxNumber + 1);
    }
    return ;
}

}
