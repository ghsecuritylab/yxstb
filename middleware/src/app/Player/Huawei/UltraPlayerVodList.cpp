
#include "UltraPlayerVodList.h"
#include "UltraPlayerAssertions.h"

#include "Message.h"
#include "MessageTypes.h"
#include "MessageHandler.h"
#include "ProgramVOD.h"
#include "BrowserPlayerReporterHuawei.h"

#include <stdio.h>
#include <string.h>

#include "mid_stream.h"


namespace Hippo {

UltraPlayerVodList::UltraPlayerVodList(UltraPlayerClient *client, BrowserPlayerReporter *pReporter, ProgramList *pProgramList, unsigned int pIndex)
	: UltraPlayerVodHuawei(client, pReporter, (ProgramVOD *)pProgramList->getProgramByIndex(pIndex))
    , mProgramList(pProgramList)
    , mCurrentIndex(pIndex)
{
}

UltraPlayerVodList::~UltraPlayerVodList()
{
    close(UltraPlayer::BlackScreenMode);
}

int
UltraPlayerVodList::PlayFirst(void)
{
    mCurrentIndex = 0;
    UltraPlayerVodHuawei::close(LastFrameMode);
    mProgram->unref();
    mProgram = (ProgramVOD *)mProgramList->getProgramByIndex(mCurrentIndex);
    if(!mProgram){
        return -1;
    }
    mProgram->ref();
    UltraPlayerVodHuawei::play(0);
    return 0;
}

int
UltraPlayerVodList::PlayLast(void)
{
    mCurrentIndex = mProgramList->getProgramCount() - 1;
    UltraPlayerVodHuawei::close(LastFrameMode);
    mProgram->unref();
    mProgram = (ProgramVOD *)mProgramList->getProgramByIndex(mCurrentIndex);
    if(!mProgram){
        return -1;
    }
    mProgram->ref();
    UltraPlayerVodHuawei::play(0);
    return 0;
}

int
UltraPlayerVodList::PlayNext(void)
{
    PLAYER_LOG("UltraPlayerVodList::PlayNext mPlayCycleFlag(%d)mPlayRandomFlag(%d)mCurrentIndex(%d)\n", mPlayCycleFlag, mPlayRandomFlag, mCurrentIndex);
    if(mPlayRandomFlag) {
        mCurrentIndex = random() % mProgramList->getProgramCount();
    } else {
        mCurrentIndex ++;
        if(mCurrentIndex >= mProgramList->getProgramCount()){
            mCurrentIndex = 0;
        }
    }
    UltraPlayerVodHuawei::close(LastFrameMode);
    mProgram->unref();
    mProgram = (ProgramVOD *)mProgramList->getProgramByIndex(mCurrentIndex);
    if(!mProgram){
        return -1;
    }
    mProgram->ref();
    UltraPlayerVodHuawei::play(0);
    return 0;
}

int
UltraPlayerVodList::PlayPrevious(void)
{
    if(mPlayRandomFlag) {
        mCurrentIndex = random() % mProgramList->getProgramCount();
    } else {
        mCurrentIndex --;
        if(mCurrentIndex < 0){
            mCurrentIndex = mProgramList->getProgramCount() - 1;
        }
    }
    UltraPlayerVodHuawei::close(LastFrameMode);
    mProgram->unref();
    mProgram = (ProgramVOD *)mProgramList->getProgramByIndex(mCurrentIndex);
    if(!mProgram){
        return -1;
    }
    mProgram->ref();
    UltraPlayerVodHuawei::play(0);
    return 0;
}

int
UltraPlayerVodList::PlayByEntryId(const char *entryID)
{
    UltraPlayerVodHuawei::close(LastFrameMode);
    mProgram->unref();
    mProgram = (ProgramVOD *)mProgramList->getProgramByStringID(entryID);
    if(!mProgram){
        return -1;
    }
    mCurrentIndex = mProgramList->getProgramIndex(mProgram);
    mProgram->ref();
    UltraPlayerVodHuawei::play(0);
    return 0;
}

int
UltraPlayerVodList::PlayByIndex(int Index)
{
    if(!mProgram){
        return -1;
    }
    mCurrentIndex = Index;
    if(mCurrentIndex < 0){
        mCurrentIndex = mProgramList->getProgramCount() - 1;
    }else if(mCurrentIndex >= mProgramList->getProgramCount()){
        mCurrentIndex = 0;
    }
    UltraPlayerVodHuawei::close(LastFrameMode);
    mProgram->unref();
    mProgram = (ProgramVOD *)mProgramList->getProgramByIndex(mCurrentIndex);
    mProgram->ref();
    UltraPlayerVodHuawei::play(0);
    return 0;
}

int
UltraPlayerVodList::PlayByOffset(int pOffset)
{
    mCurrentIndex += pOffset;
    if(mCurrentIndex < 0) {
        mCurrentIndex += mProgramList->getProgramCount();
    } else if(mCurrentIndex >= mProgramList->getProgramCount()) {
        mCurrentIndex -= mProgramList->getProgramCount();
    }
    UltraPlayerVodHuawei::close(LastFrameMode);
    mProgram->unref();
    mProgram = (ProgramVOD *)mProgramList->getProgramByIndex(mCurrentIndex);
    if(!mProgram){
        return -1;
    }
    mProgram->ref();
    UltraPlayerVodHuawei::play(0);
    return 0;
}

int
UltraPlayerVodList::getVodListCurIndex(void)
{
    return mCurrentIndex;
}

void
UltraPlayerVodList::handleMessage(Message *msg)
{
    if(msg->what >= MessageType_ConfigSave && msg->what <= MessageType_ClearAllIcon){
        return UltraPlayer::handleMessage(msg);
    }
    PLAYER_LOG("UltraPlayerVodList receive message what(0x%x),info(0x%x & 0x%x)\n", msg->what, msg->arg1, msg->arg2);
    if(msg->arg1 >= 0x1000){
        switch(msg->arg1 - 0x1000){
            case STRM_STATE_PAUSE:{
                UltraPlayer::ShowPlayStateIcon(PlayStateWidget::StatePause);
                break;
            }
            case STRM_STATE_PLAY:{
                UltraPlayer::ShowPlayStateIcon(PlayStateWidget::StatePlay);
                break;
            }
            case STRM_STATE_FAST:{
                switch(msg->arg2){
                    case 2:{
                        UltraPlayer::ShowPlayStateIcon(PlayStateWidget::StateFW2);
                        break;
                    }
                    case 4:{
                        UltraPlayer::ShowPlayStateIcon(PlayStateWidget::StateFW4);
                        break;
                    }
                    case 8:{
                        UltraPlayer::ShowPlayStateIcon(PlayStateWidget::StateFW8);
                        break;
                    }
                    case 16:{
                        UltraPlayer::ShowPlayStateIcon(PlayStateWidget::StateFW16);
                        break;
                    }
                    case 32:{
                        UltraPlayer::ShowPlayStateIcon(PlayStateWidget::StateFW32);
                        break;
                    }
                    case -2:{
                        UltraPlayer::ShowPlayStateIcon(PlayStateWidget::StateRW2);
                        break;
                    }
                    case -4:{
                        UltraPlayer::ShowPlayStateIcon(PlayStateWidget::StateRW4);
                        break;
                    }
                    case -8:{
                        UltraPlayer::ShowPlayStateIcon(PlayStateWidget::StateRW8);
                        break;
                    }
                    case -16:{
                        UltraPlayer::ShowPlayStateIcon(PlayStateWidget::StateRW16);
                        break;
                    }
                    case -32:{
                        UltraPlayer::ShowPlayStateIcon(PlayStateWidget::StateRW32);
                        break;
                    }
                    default:
                        break;
                }
                break;
            }
            default:{
                break;
            }
        }
        mCurrentStatus = msg->arg1 - 0x1000;
        mCurrentSpeed = msg->arg2;
        reporter()->reportState((STRM_STATE)mCurrentStatus, mCurrentSpeed);
    }
    else {
        switch(msg->arg1) {
            case STRM_MSG_STREAM_VIEW: {
                UltraPlayer::ShowPlayStateIcon(PlayStateWidget::StatePlay);
                UltraPlayer::handleMessage(msg);
                break;
            }
            case STRM_MSG_STREAM_END: {
                if(PlayNext() == 0) {
                    UltraPlayer::ShowPlayStateIcon(PlayStateWidget::StateNext);
                }
                else {
                    UltraPlayer::ShowPlayStateIcon(PlayStateWidget::StateStop);
                }
                break;
            }
            default:
                break;
        }
        reporter()->reportMessage((STRM_MSG)msg->arg1, msg->arg2);
    }
}

} // namespace Hippo

