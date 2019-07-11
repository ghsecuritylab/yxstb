#pragma once 

#ifdef __cplusplus

#include "Serialization.h"
#include <list>
#include <map>
#include <algorithm>
#include "ProgramChannelC10.h"

namespace Hippo {

class ProgramFinger : public Serialization
{
public:
    static ProgramFinger* GetInstance();
    virtual ~ProgramFinger();

    virtual void Serialize(Archive& ar);
    virtual void Deserialize(Archive& ar);

    // 打开这个功能！
    void Enable();
    void Disable();

public:
    void Save();

    // ProgramChannel类一些函数没有多态。。所以只能这么传了。
    void joinChannel(ProgramChannelC10* p);

    // 看到第一幅画面。
    void onPtsView();

    // 可以考虑定时来刷新观看时间并保存，免得在看某个频道时断电。
    void onUpdate();

    // 观看结束，统计一些信息。
    void leaveChannel();

   
    // You can use either a function or a functor.
    // CallbackFunctor: void CallbackFunctor(ProgramFinger::Node* node);
    template <class CallbackFunctor>
    void EnumNodes(CallbackFunctor& func) {
        if (mNode.empty())
            return;
        std::for_each(mNode.begin(), mNode.end(), func); 
    }

    void SetCurrentBandwidth(int b);
    void SetCurrentBitRate(int rate);

public:
    class Node : public Serialization {
    public:
        Node() : mUserChannelID(0),
            Jitter(0), MdiMLR(0), MdiDF(0), 
            mChannelRequestFrequency(0),
            mAccessSuccessNumber(0),
            mTotalAccessTime(0),
            mWatchLong(0),
            mMediaStreamBandwidth(0) {}

        virtual void Serialize(Archive& ar);
        virtual void Deserialize(Archive& ar);

        int getAverageAccessTime()
        {
            if (mAccessSuccessNumber == 0) {
                return 0;
            }
            return mTotalAccessTime / mAccessSuccessNumber;
        }

        int mUserChannelID;
        std::string mChannelName;
        std::string mChannelAddress;
        std::string mTransmission;
        std::string mProgramStartTime;
        std::string mProgramEndTime;
        unsigned int Jitter;
        unsigned int MdiMLR;
        unsigned int MdiDF;
        std::string mBitRate;
        std::string mVideoQuality;
        int mChannelRequestFrequency;
        int mAccessSuccessNumber;
        // 不计录平均时间，而记录总时间。需要时计算出平均时间来。
        // int mAverageAccessTime;
        int mTotalAccessTime;
        int mWatchLong;
        int mMediaStreamBandwidth;

        void Print();
    };

private:
    ProgramFinger();

    void PrintNodes();
    void Test();
 
    void reset();
    Node* createNode();
    Node* findNodeByChannelName(const char * name);
    long now();
    void CalcWatchLong();
    void CalcSqmParameter();

    Node* mCurrentNode;
    std::list<Node*> mNode;
    std::map<std::string, Node*>    mMap;
    long mStartTime;
    long mLastWatchTime;
    bool mEnabled;

};

} // namespace Hippo

extern "C" void EnableProgramFinger();
extern "C" void ProgramFinger_SetCurrentBandwidth(int b);
extern "C" void ProgramFinger_SetCurrentBitRate(int rate);
#else // ifdef __cplusplus
void EnableProgramFinger();
void ProgramFinger_SetCurrentBandwidth(int b);
void ProgramFinger_SetCurrentBitRate(int rate);
#endif

