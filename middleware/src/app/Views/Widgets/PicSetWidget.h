#ifndef _PICSETWIDGET_H_
#define _PICSETWIDGET_H_

#include "Widget.h"
#ifdef __cplusplus

#include <iostream>
#include <vector>

namespace Hippo {

class PicSetWidget : public Widget {
  public:
      PicSetWidget(WidgetSource* nSource, int nMaxNumber);

      ~PicSetWidget();

      int getMaxNumber( ) { return mMaxNumber; }
      void setOrderNumber(int nOrderID);
      int getOrderNumber( ) { return mOrderNumber; }

      void hide( ) { mOrderNumber = 0; setVisibleP(false); }
      void show( ) { inval(NULL); setVisibleP(true); }
      void insert(int nOrderID, void* nImageAddress, int nImageLength);
  protected:
      /** Override this to draw inside the view. Be sure to call the inherited version too */
      virtual void onDraw(Canvas*);

  private:
      int mMaxNumber; /* start from 0 */
      int mOrderNumber; /* start from 0 */
      std::vector<void *> mImagesAddr;
      std::vector<int> mImagesLen;
};

} // namespace Hippo

#endif
#endif
