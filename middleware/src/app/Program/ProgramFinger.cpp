

#include "ProgramFinger.h"
#include <sys/sysinfo.h>
#include "iptv_logging.h"

#include "tr069_port_ctc.h"

class TestHelper {
public:
    void operator()(Hippo::ProgramFinger::Node* node) {
        RUN_HERE();
        node->Print();
    }
};

namespace Hippo {

static ProgramFinger* g_instance = NULL;

ProgramFinger::ProgramFinger()
    : mCurrentNode(NULL)
    , mStartTime(0)
    , mLastWatchTime(0)
    , mEnabled(false)
{
    // LoadFromFile("/root/program.dat");
}

ProgramFinger::~ProgramFinger()
{
}

ProgramFinger* ProgramFinger::GetInstance()
{
    if (!g_instance) {
        g_instance = new ProgramFinger();
        // g_instance->Enable();
    }
    return g_instance;
}

void ProgramFinger::Enable()
{
    LoadFromFile("/root/program.dat");
    mEnabled = true;
}

void ProgramFinger::Disable()
{
    leaveChannel();
    unlink("/root/program.dat");
    mEnabled = false;
}

void ProgramFinger::Save()
{
    SaveToFile("/root/program.dat");
}

void ProgramFinger::Serialize(Archive& ar)
{
    std::string magic("ProgramFinger1.0");
    ar << magic << mNode.size();

    ArchiveSerializationFunctor asf(ar);
    for_each(mNode.begin(), mNode.end(), asf);
}

void ProgramFinger::Deserialize(Archive& ar)
{
    reset();
    std::string magic;
    ar >> magic;
    if (magic != "ProgramFinger1.0") {
        return;
    }
    size_t count = 0;
    ar >> count;
    ArchiveDeserializationFunctor adf(ar);

    size_t i;
    for (i = 0; i < count; i++) {
        Node* n = createNode();
        adf(n);
        mMap[n->mChannelName] = n;
    }
}

void ProgramFinger::Node::Serialize(Archive& ar)
{
    ar << mUserChannelID << mChannelName << mChannelAddress << mTransmission << mProgramStartTime << mProgramEndTime << Jitter << MdiMLR << MdiDF << mBitRate << mVideoQuality << mChannelRequestFrequency << mAccessSuccessNumber << mTotalAccessTime << mWatchLong << mMediaStreamBandwidth;
}

void ProgramFinger::Node::Deserialize(Archive& ar)
{
    ar >> mUserChannelID >> mChannelName >> mChannelAddress >> mTransmission >> mProgramStartTime >> mProgramEndTime >> Jitter >> MdiMLR >> MdiDF >> mBitRate >> mVideoQuality >> mChannelRequestFrequency >> mAccessSuccessNumber >> mTotalAccessTime >> mWatchLong >> mMediaStreamBandwidth;
}

void ProgramFinger::Node::Print()
{
    RUN_HERE() << "";
    RUN_HERE() << "UserChannelID = " << mUserChannelID;
    RUN_HERE() << "ChannelName = " << mChannelName;
    RUN_HERE() << "ChannelAddress = " << mChannelAddress;
    RUN_HERE() << "Transmission = " << mTransmission;
    RUN_HERE() << "StartTime = " << mProgramStartTime;
    RUN_HERE() << "EndTime = " << mProgramEndTime;
    RUN_HERE() << "Jitter = " << Jitter;
    RUN_HERE() << "MdiMLR = " << MdiMLR;
    RUN_HERE() << "MdiDF = " << MdiDF;
    RUN_HERE() << "BitRate = " << mBitRate;
    RUN_HERE() << "VideoQuality = " << mVideoQuality;
    RUN_HERE() << "ChannelRequestFrequency = " << mChannelRequestFrequency;
    RUN_HERE() << "AccessSuccessNumber = " << mAccessSuccessNumber;
    RUN_HERE() << "TotalAccessTime = " << mTotalAccessTime;
    RUN_HERE() << "WatchLong = " << mWatchLong;
    RUN_HERE() << "MediaStreamBandwidth = " << mMediaStreamBandwidth;
}

ProgramFinger::Node* ProgramFinger::findNodeByChannelName(const char * name)
{
    std::string n(name);
    if (mMap.find(n) != mMap.end())
        return mMap[n];
    return NULL;
}

void ProgramFinger::joinChannel(ProgramChannelC10* p)
{
    if (!mEnabled)
        return;

    if (mCurrentNode)
        leaveChannel();
    Node* n = findNodeByChannelName(p->GetChanName().c_str());
    if (!n) {
        n = createNode();
        std::string name(p->GetChanName().c_str());
        mMap[name] = n;
    }
    n->mUserChannelID = p->GetUserChanID();
    n->mChannelName = p->GetChanName().c_str();
    n->mChannelAddress = p->GetChanURL().c_str();
    n->mTransmission = (strncmp(p->GetChanURL().c_str(), "igmp://", 7) == 0) ? "1" : "0";
    char    buff[2048];
    snprintf(buff, sizeof(buff), "%d", p->GetBeginTime());
    n->mProgramStartTime = buff;
    snprintf(buff, sizeof(buff), "%d", p->GetBeginTime() + p->GetLasting());
    n->mProgramEndTime = buff;
    n->mChannelRequestFrequency++;
    mStartTime = now();
    mLastWatchTime = 0;
    mCurrentNode = n;
}

void ProgramFinger::onPtsView()
{
    if (!mEnabled)
        return;

    if (!mCurrentNode)
        return;
    mCurrentNode->mAccessSuccessNumber++;

    long ut = now() - mStartTime;
    if (mStartTime > 0 && ut > 0) {
        mCurrentNode->mTotalAccessTime += ut;
    }
    mLastWatchTime = now();
}

void ProgramFinger::onUpdate()
{
    if (!mEnabled)
        return;

    if (!mCurrentNode)
        return;

    CalcWatchLong();
    Save();
}

void ProgramFinger::leaveChannel()
{
    if (!mEnabled)
        return;

    CalcWatchLong();
    CalcSqmParameter();

    mCurrentNode = NULL;
    Save();
    // PrintNodes();
    // Test();
}

void ProgramFinger::CalcWatchLong()
{
    if (!mCurrentNode)
        return;
    long ut = now() - mLastWatchTime;
    if (mLastWatchTime > 0 && ut > 0)
        mCurrentNode->mWatchLong += ut;
}

void ProgramFinger::SetCurrentBandwidth(int b)
{
    if (!mCurrentNode)
        return;
    mCurrentNode->mMediaStreamBandwidth = b;
}

void ProgramFinger::SetCurrentBitRate(int rate)
{
    if (!mCurrentNode)
        return;
    float r = (float)rate / 1024.0f / 1024.0f;
    char    temp[100];
    snprintf(temp, sizeof(temp), "%.2f Mbps", r);
    mCurrentNode->mBitRate = temp;
}

void ProgramFinger::CalcSqmParameter()
{
    if (!mCurrentNode)
        return;
    mCurrentNode->Jitter = app_tr069_port_Monitor_get_Jitter();
    mCurrentNode->MdiDF = app_tr069_port_Monitor_get_MdiDF();
    mCurrentNode->MdiMLR = app_tr069_port_Monitor_get_MdiMLR();
}

void ProgramFinger::PrintNodes()
{
    ILOG() << TERMC_PINK << "-------------------------------------------------------" << TERMC_NONE;
    std::list<Node*>::iterator it;
    for (it = mNode.begin(); it != mNode.end(); it++) {
        (*it)->Print();
    }
}

void ProgramFinger::Test()
{
    TestHelper t;
    EnumNodes(t);
}

ProgramFinger::Node* ProgramFinger::createNode()
{
    Node* n = new Node();
    mNode.push_back(n);
    return n;
}

void ProgramFinger::reset()
{
    std::list<Node*>::iterator it;
    for (it = mNode.begin(); it != mNode.end(); it++) {
        delete *it;
    }
    mNode.clear();
    mMap.clear();
    mCurrentNode = NULL;
}

long ProgramFinger::now()
{
    struct sysinfo info;
    sysinfo(&info);
    return info.uptime;
}

} // namespace Hippo


void EnableProgramFinger()
{
    Hippo::ProgramFinger::GetInstance()->Enable();
}

void ProgramFinger_SetCurrentBandwidth(int b)
{
    Hippo::ProgramFinger::GetInstance()->SetCurrentBandwidth(b);
}

void ProgramFinger_SetCurrentBitRate(int rate)
{
    Hippo::ProgramFinger::GetInstance()->SetCurrentBitRate(rate);
}











