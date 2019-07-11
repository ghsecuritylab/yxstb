#ifndef __NetworkSpeedDiagnose__H_
#define __NetworkSpeedDiagnose__H_

#include "NetworkDiagnose.h"
#include <time.h>

#ifdef __cplusplus
#include <string>
#include <vector>

class NetworkSpeedDiagnose : public NetworkDiagnose::DiagnoseProcess {
public:
    NetworkSpeedDiagnose();
    ~NetworkSpeedDiagnose(); 

    virtual int type() { return eTypeNetworkSpeedDiagnose; }
    virtual int start(NetworkDiagnose* netdiag);
    virtual int stop();
    virtual int doResult(int result, int size, void* data);

    void setTestUrl(const char* url) { mTestUrl = url; }
    void setSamplePeriod(int time) { mSamplePeriod = time; }
    void setAutoStopTime(int time) { mAutoStopTime = time; }

    double getMaxSpeed() { return mMaxSpeed; }
    double getMinSpeed() { return mMinSpeed; }
    double getAverageSpeed() { return mAverageSpeed; }
    
protected:
    int _SpeedTest(); 

private:
    std::string mTestUrl; 
    int mSamplePeriod; //10s
    int mAutoStopTime;
    long mSampleRecvSize; 

    time_t mTestST;
    time_t mTestET;
    time_t mRecordPoint;

    double mMaxSpeed;
    double mMinSpeed;
    double mAverageSpeed;

    std::vector<double> mSampleSpeed;
};
#endif

#endif
