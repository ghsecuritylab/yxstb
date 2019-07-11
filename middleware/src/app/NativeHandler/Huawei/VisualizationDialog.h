#ifndef VisualizationDialog_H_
#define VisualizationDialog_H_

#include "Dialog.h"
#include "VisualizationInfoWidget.h"
#include "MaintenancePageWidget.h"



#ifdef __cplusplus

namespace Hippo {

class NativeHandlerPublic;

class VisualizationDialog : public Dialog {
public:
    VisualizationDialog();
    ~VisualizationDialog();

    enum CurrentFocusLine {
        Unknown_line = 0,
        MainPage_CollectSTBDebugInfo_Line,
        MainPage_AutoCollectSTBDebugInfo_Line,
        MainPage_ShowStreamMediaInfo_Line,
        MainPage_ShowOTTDebugInfo_Line,
        DebugPage_Start_line,
        DebugPage_Stop_line,
    };

    enum CurrentPageInfo {
        UnknownPage = 0,
        MainPage,
        DebugPage,
        AutoDebugPage,
        StreamInfoPage,
        OTTInfoPage,
        NULLBGPage,
    };

    void setHandler(NativeHandlerPublic *handler);
    virtual bool handleMessage(Message *);
    virtual void draw();

protected:
    NativeHandlerPublic* m_handle;
    bool doRefreshVisualizationInfo();
    bool doStopDebug();
    bool doPressUP();
    bool doPressDown();
    bool doPressSelect();
    bool doPressGoBack();


private:
    VisualizationInfoWidget *mVisualizationInfo;
    MaintenancePageWidget *mMaintenancePage;
    bool isOpenMainPage;
    bool isOpenChildPage;
    int mCurrentPage;
    int mCurrentFocusPos;

};

} // namespace Hippo

#endif // __cplusplus

#endif // VsualizationDialog_H_
