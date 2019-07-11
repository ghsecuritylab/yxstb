
#include <time.h>

#include "ProgressBarWidget.h"
#include "ViewAssertions.h"

#include "Canvas.h"

#include "mid/mid_time.h"
#include "mid/mid_tools.h"
#include "ind_tmr.h"

#include "SysSetting.h"
#include "browser_event.h"

#define GIF_SEEK_BG_PATH           SYS_IMG_PATH_ROOT"/playstate/seek/seek_bg.gif"
#define GIF_SEEK_BG_1_PATH         SYS_IMG_PATH_ROOT"/playstate/seek/seek_bg1.gif"
#define GIF_PROGRESS_BG_PATH       SYS_IMG_PATH_ROOT"/playstate/seek/progress_bg.gif"
#define GIF_PROGRESS_SLASH_PATH    SYS_IMG_PATH_ROOT"/playstate/seek/progress_slash.gif"

namespace Hippo
{

ProgressBarWidget::ProgressBarWidget(WidgetSource *source)
    : Widget(source)
    , mProgress(0)
    , mState(StateSeek)
    , mFocusPos(PosSeeker)
    , mTimeL(0)
    , mTimeR(0)
    , mTimeC(0)
    , mLastPos(PosHourL)
{
    VIEW_LOG("===Current source image path(%s),w=[%d],h=[%d],x=[%d],y=[%d]\n", mSource->image, mSource->w, mSource->h, mSource->x, mSource->y);
    memset(mMaskH, 0, sizeof(mMaskH));
    memset(mMaskM, 0, sizeof(mMaskM));
    memset(mMaskS, 0, sizeof(mMaskS));
}

ProgressBarWidget::~ProgressBarWidget()
{
}

int
ProgressBarWidget::setProgress(int progress)
{
    int oldProgress = mProgress;
    mProgress = progress;
    inval(NULL);
    return oldProgress;
}

int
ProgressBarWidget::getProgress()
{
    return mProgress;
}

int
ProgressBarWidget::setState(State s)
{
    mState = s;
    inval(NULL);
    return 0;
}

ProgressBarWidget::State
ProgressBarWidget::getState()
{
    return mState;
}

void
ProgressBarWidget::setCurrentTime(unsigned int _t)
{
    VIEW_LOG("Current time(%u)\n", _t);
    mTimeC = _t;
    memset(mMaskH, 0, sizeof(mMaskH));
    memset(mMaskM, 0, sizeof(mMaskM));
    memset(mMaskS, 0, sizeof(mMaskS));
}

int
ProgressBarWidget::setFocusPos(FocusPos pos)
{
    mFocusPos = pos;
    return 0;
}

ProgressBarWidget::FocusPos
ProgressBarWidget::getFocusPos()
{
    return mFocusPos;
}

void
ProgressBarWidget::onDraw(Canvas* canvas)
{
    const void * imgSeekBg = GIF_SEEK_BG_PATH;
    const void * imgProgressBg = GIF_PROGRESS_BG_PATH;
    const void * imgProgressSlash = GIF_PROGRESS_SLASH_PATH;


    if(mState == StateSeek || mState == StateSeekError) {/*{{{*/
        if(mState == StateSeekError){
			imgSeekBg = GIF_SEEK_BG_1_PATH;
		}

        Rect bounds;
        getLocalBounds(&bounds);
        VIEW_LOG("drawImage [%s] start\n", (unsigned char*)imgSeekBg);
		{		
			Image img((unsigned char*)imgSeekBg); 
			drawImage(canvas, &bounds, &img); //draw	
		} 

        AutoCanvasRestore acr(canvas, true);
        cairo_scale(canvas->mCairo, (double)width() / (double)(mSource->w), (double)height() / (double)(mSource->h));

        Rect rcL, rcR, rcH, rcM, rcS;
        rcL.set(20, 15, 128, 32);
        rcR.set(512, 15, 620, 32);
        rcH.set(294, 50, 322, 70);
        rcM.set(336, 50, 364, 70);
        rcS.set(378, 50, 406, 70);

        VIEW_LOG("rcL: (%d, %d, %d, %d)\n", rcL.fLeft, rcL.fTop, rcL.fRight, rcL.fBottom);

        char mTextL[32], mTextR[32], mTextC[32];

        VIEW_LOG("StartTime(%d), EndTime(%d), CurrentTime(%d)\n", mTimeL, mTimeR, mTimeC);
        VIEW_LOG("CurrentUTCTime(%d)\n", time(NULL));
        if(MID_UTC_SUPPORT) {
            int time_zone = 0;

            sysSettingGetInt("timezone", &time_zone, 0);
            if(mTimeR == time(NULL)) {
                mTimeL += time_zone * 60 * 60;
                mTimeR += time_zone * 60 * 60;
                mTimeC += time_zone * 60 * 60;
            } else {
                if(mTimeC > mTimeR || mTimeC < mTimeL) {
                    mTimeC += time_zone * 60 * 60;
                }
            }
        }
        if(mTimeC < mTimeL)
            mTimeC = mTimeL;
        mid_tool_time2string(mTimeL, mTextL, ':');
        mid_tool_time2string(mTimeR, mTextR, ':');
        mid_tool_time2string(mTimeC, mTextC, ':');
        VIEW_LOG("StartTime(%s), EndTime(%s), CurrentTime(%s)\n", mTextL, mTextR, mTextC);
        mTextC[13] = '\0';
        mTextC[16] = '\0';

        drawText(canvas, &rcL, mTextL + 11);
        drawText(canvas, &rcR, mTextR + 11);
        if(mMaskH[0] != '\0')
            drawText(canvas, &rcH, mMaskH);
        else
            drawText(canvas, &rcH, mTextC + 11);

        if(mMaskM[0] != '\0')
            drawText(canvas, &rcM, mMaskM);
        else
            drawText(canvas, &rcM, mTextC + 14);

        if(mMaskS[0] != '\0')
            drawText(canvas, &rcS, mMaskS);
        else
            drawText(canvas, &rcS, mTextC + 17);

        Rect        rect;
        Point       pt;
        WidgetColor color(192, 192, 192, 255);
        int         w = 500;

        VIEW_LOG("R = [%u], L = [%u], C = [%u]\n", mTimeR, mTimeL, mTimeC);
        if(mTimeR - mTimeL > 0) {
            w = 140 + (500 - 140) * (mTimeC - mTimeL) / (mTimeR - mTimeL);
        }
        rect.set(140, 20, w, 29);
        Rectangle(canvas, rect, WidgetColor(0, 255, 128, 255));
        pt.set(w, 25);

        switch(mFocusPos) {
        case PosSeeker:
            color = WidgetColor(255, 255, 0, 255);
            break;
        case PosHourL:
            rect.set(298, 50, 308, 69);
            break;
        case PosHourR:
            rect.set(308, 50, 318, 69);
            break;
        case PosMinL:
            rect.set(340, 50, 350, 69);
            break;
        case PosMinR:
            rect.set(350, 50, 360, 69);
            break;
        case PosSecL:
            rect.set(382, 50, 392, 69);
            break;
        case PosSecR:
            rect.set(392, 50, 402, 69);
            break;
        default:
            break;
        }
        Arc(canvas, pt, 8, color);
        if(mFocusPos != PosSeeker)
            Rectangle(canvas, rect, WidgetColor(255, 0, 0, 128));
    } /*}}}*/
    else if(mState == StateProgress) {
        Rect    bounds;
        getLocalBounds(&bounds);
        bounds.fBottom = bounds.fTop + 60;
		VIEW_LOG("drawImage [%s] start\n", (unsigned char*)imgProgressBg);
		{		
			Image img((unsigned char*)imgProgressBg); 
			drawImage(canvas, &bounds, &img); //draw	
		} 	

        AutoCanvasRestore acr(canvas, true);
        cairo_scale(canvas->mCairo, (double)width() / (double)(mSource->w), (double)height() / (double)(mSource->h));

        Rect rcL, rcR, rcC;
        rcL.set(20, 24, 128, 44);
        rcR.set(512, 24, 620, 44);
        rcC.set(346, 4, 260, 24);

        if(MID_UTC_SUPPORT) {
            int time_zone = 0;

            sysSettingGetInt("timezone", &time_zone, 0);
            if(mTimeR == time(NULL)) {
                mTimeL += time_zone * 60 * 60;
                mTimeR += time_zone * 60 * 60;
                mTimeC += time_zone * 60 * 60;
            } else {
                if(mTimeC > mTimeR || mTimeC < mTimeL) {
                    mTimeC += time_zone * 60 * 60;
                }
            }
        }
        VIEW_LOG("StartTime(%u) EndTime(%u) CurrentTime(%u)\n", mTimeL, mTimeR, mTimeC);
        char mTextL[100] = {0}, mTextR[100] = {0}, mTextC[100] = {0};
        char Progress[8] = {0};

        mid_tool_time2string(mTimeL, mTextL, ':');
        mid_tool_time2string(mTimeR, mTextR, ':');
        mid_tool_time2string(mTimeC, mTextC, ':');
        sprintf(Progress, "/%d", ((mTimeC - mTimeL) * 100/(mTimeR - mTimeL)));
        strcat(mTextC, Progress);
        strcat(mTextC, "%");
        // VIEW_LOG("drawText: %s, %s.\n", mTextL, mTextR);
        drawText(canvas, &rcL, mTextL + 11);
        drawText(canvas, &rcR, mTextR + 11);
        drawText(canvas, &rcC, mTextC + 11);

        Rect        rect;
        Point       pt;
        WidgetColor color(192, 192, 192, 255);
        int         w = 500;

        // VIEW_LOG("R = [%u], L = [%u], C = [%u]\n", mTimeR, mTimeL, mTimeC);
        if(mTimeR - mTimeL > 0) {
            w = 140 + (500 - 140) * (mTimeC - mTimeL) / (mTimeR - mTimeL);
        }
        rect.set(140, 30, w, 39);
        Rectangle(canvas, rect, WidgetColor(0, 255, 128, 255));
        pt.set(w, 34);
        Arc(canvas, pt, 8, color);
    }
}

bool
ProgressBarWidget::InputKey(int key)
{
    FocusPos pos = getFocusPos();

    if(mState == StateSeekError)
        mState = StateSeek;

    if(mState == StateSeek) {
        switch(pos) {
        case ProgressBarWidget::PosSeeker:
            switch(key) {
            case EIS_IRKEY_LEFT: {
                unsigned int step = (mTimeR - mTimeL) / 10;
                if(mTimeC < mTimeL + step)
                    mTimeC = mTimeL;
                else
                    mTimeC -= step;
                setCurrentTime(mTimeC);
                break;
            }
            case EIS_IRKEY_RIGHT: {
                mTimeC += (mTimeR - mTimeL) / 10;
                if(mTimeC > mTimeR)
                    mTimeC = mTimeR;
                setCurrentTime(mTimeC);
                break;
            }
            case EIS_IRKEY_DOWN:
                setFocusPos(mLastPos);
                break;
            case EIS_IRKEY_SELECT:
                return true;        // ¿ÉÒÔÌø×ª¡£
            default:
                return true;
            }
            inval(NULL);
            return true;
            break;
        case ProgressBarWidget::PosHourL:
        case ProgressBarWidget::PosHourR:
        case ProgressBarWidget::PosMinL:
        case ProgressBarWidget::PosMinR:
        case ProgressBarWidget::PosSecL:
        case ProgressBarWidget::PosSecR:
            switch(key) {
            case EIS_IRKEY_UP:
                mLastPos = pos;
                setFocusPos(ProgressBarWidget::PosSeeker);
                break;
            case EIS_IRKEY_LEFT:
                if(pos == ProgressBarWidget::PosHourL)
                    pos = ProgressBarWidget::PosHourL;
                else if(pos == ProgressBarWidget::PosHourR)
                    pos = ProgressBarWidget::PosHourL;
                else if(pos == ProgressBarWidget::PosMinL)
                    pos = ProgressBarWidget::PosHourR;
                else if(pos == ProgressBarWidget::PosMinR)
                    pos = ProgressBarWidget::PosMinL;
                else if(pos == ProgressBarWidget::PosSecL)
                    pos = ProgressBarWidget::PosMinR;
                else if(pos == ProgressBarWidget::PosSecR)
                    pos = ProgressBarWidget::PosSecL;
                else
                    return true;
                setFocusPos(pos);
                break;
            case EIS_IRKEY_RIGHT:
                if(pos == ProgressBarWidget::PosHourL)
                    pos = ProgressBarWidget::PosHourR;
                else if(pos == ProgressBarWidget::PosHourR)
                    pos = ProgressBarWidget::PosMinL;
                else if(pos == ProgressBarWidget::PosMinL)
                    pos = ProgressBarWidget::PosMinR;
                else if(pos == ProgressBarWidget::PosMinR)
                    pos = ProgressBarWidget::PosSecL;
                else if(pos == ProgressBarWidget::PosSecL)
                    pos = ProgressBarWidget::PosSecR;
                else if(pos == ProgressBarWidget::PosSecR)
                    pos = ProgressBarWidget::PosSecR;
                else
                    return true;
                setFocusPos(pos);
                break;
            case EIS_IRKEY_NUM0:
            case EIS_IRKEY_NUM1:
            case EIS_IRKEY_NUM2:
            case EIS_IRKEY_NUM3:
            case EIS_IRKEY_NUM4:
            case EIS_IRKEY_NUM5:
            case EIS_IRKEY_NUM6:
            case EIS_IRKEY_NUM7:
            case EIS_IRKEY_NUM8:
            case EIS_IRKEY_NUM9: {
                char mTextC[100];
                mid_tool_time2string(mTimeC, mTextC, ':');
                mTextC[13] = '\0';
                mTextC[16] = '\0';
                if(mMaskH[0] == '\0')
                    strncpy(mMaskH, mTextC + 11, 2);
                if(mMaskM[0] == '\0')
                    strncpy(mMaskM, mTextC + 14, 2);
                if(mMaskS[0] == '\0')
                    strncpy(mMaskS, mTextC + 17, 2);

                char    c = '0' + key - EIS_IRKEY_NUM0;
                if(pos == PosHourL)
                    mMaskH[0] = c;
                else if(pos == PosHourR)
                    mMaskH[1] = c;
                else if(pos == PosMinL)
                    mMaskM[0] = c;
                else if(pos == PosMinR)
                    mMaskM[1] = c;
                else if(pos == PosSecL)
                    mMaskS[0] = c;
                else if(pos == PosSecR)
                    mMaskS[1] = c;
                break;
            }
            case EIS_IRKEY_SELECT: {
                if((mMaskM[0] != '\0' && atoi(mMaskM) > 60) || (mMaskS[0] != '\0' && atoi(mMaskS) > 60)) {
                    mState = StateSeekError;
                    inval(NULL);
                    return false;
                }
                struct ind_time t;
                ind_time_local(mTimeC, &t);
                if(mMaskH[0] != '\0')
                    t.hour = atoi(mMaskH);
                if(mMaskM[0] != '\0')
                    t.min = atoi(mMaskM);
                if(mMaskS[0] != '\0')
                    t.sec = atoi(mMaskS);
                unsigned int tmp = ind_time_make(&t);
                if(tmp < mTimeL || tmp > mTimeR) {
                    mState = StateSeekError;
                    inval(NULL);
                    return false;
                }
                setCurrentTime(tmp);
                return true;
            }
            default:
                return true;
            }
            inval(NULL);
            return true;
            break;
        default:
            return false;
            break;
        }
    }
    return true;
}

} // namespace Hippo


