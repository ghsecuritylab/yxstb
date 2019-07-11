#include "NetworkAssertions.h"
#include "NetworkSpeedDiagnose.h"

#include <curl/curl.h>

static size_t _WriteCallback(void *ptr, size_t size, size_t nmemb, void *data);

NetworkSpeedDiagnose::NetworkSpeedDiagnose() 
    : mSampleRecvSize(0), mRecordPoint(0)
    , mMaxSpeed(0.0), mMinSpeed(0.0), mAverageSpeed(0.0)
{
}

NetworkSpeedDiagnose::~NetworkSpeedDiagnose()
{
    while (mDiagState != eFinish)
        usleep(100000);
}

int
NetworkSpeedDiagnose::_SpeedTest() 
{
    NETWORK_LOG_INFO("TestUrl:%s\n", mTestUrl.c_str());
    int status = eTEST_TIMEOUT, ret = -1, errcode = 0, i = 0;
    double sum = 0.0;
    CURL* easy = 0;

    if (std::string::npos == mTestUrl.find("http"))
        return eHTTP_RESPONSE;

    mTestST = time(0);
    mTestET = mTestST + mAutoStopTime;

    easy = curl_easy_init();
    if (!easy)
        return eTEST_ERR;

    curl_easy_setopt(easy, CURLOPT_URL, mTestUrl.c_str());
    curl_easy_setopt(easy, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(easy, CURLOPT_CONNECTTIMEOUT, 3);
    curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, _WriteCallback);
    curl_easy_setopt(easy, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(easy, CURLOPT_VERBOSE, 1L);//debug set

    ret = curl_easy_perform(easy);
    switch (ret) {
    case CURLE_OK:
        curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE, &errcode);
        switch (errcode) {
        case 403: //Forbidden
            status = eHTTP_REJECT;
            break;
        case 404: //Not Found
            status = eHTTP_RESPONSE;
            break;
        case 200: //Http Ok
            status = eTEST_OK;
            break;
        default:
            NETWORK_LOG_INFO("http errcode:%d\n", errcode);
            status = eHTTP_UNKNOW;
        }
        break;
    case CURLE_COULDNT_RESOLVE_HOST: //6
        status = eDNS_ERR;
        break;
    case CURLE_OPERATION_TIMEDOUT: //28
        status = eHTTP_TIMEOUT;
        break;
    case CURLE_WRITE_ERROR: //stop or timeout.
        status = eTEST_TIMEOUT;
        break;
    default:
        NETWORK_LOG_INFO("curl_easy_perform(): %d\n", ret);
        status = eTEST_ERR;
        break;
        ;
    }
    curl_easy_cleanup(easy);

    return mDiagState == eRun ? status : eTEST_STOP;
}

int 
NetworkSpeedDiagnose::start(NetworkDiagnose* netdiag)
{
    NETWORK_LOG_INFO("Start:%s\n", netdiag->getTestType());

    mDiagState = eRun;

    int status = _SpeedTest();

    switch (status) {
    case eTEST_OK:
        netdiag->setTestState(ND_STATE_FINISH);
        netdiag->setTestResult(ND_RESULT_SUCCESS);
        break;
    case eTEST_TIMEOUT:
        netdiag->setTestState(ND_STATE_AUTOSTOP);
        netdiag->setTestResult(ND_RESULT_SUCCESS);
        break;
    case eTEST_STOP:
        netdiag->setTestState(ND_STATE_MANUSTOP);
        netdiag->setTestResult(ND_RESULT_SUCCESS);
        break;
    case eHTTP_RESPONSE:
        netdiag->setTestState(ND_STATE_FAIL);
        netdiag->setErrorCode(102050);
        break;
    case eDNS_ERR:
    case eHTTP_REJECT:
        netdiag->setTestState(ND_STATE_FAIL);
        netdiag->setErrorCode(102051);
        break;
    case eTEST_ERR:
    case eHTTP_UNKNOW:
    default:
        netdiag->setTestState(ND_STATE_FAIL);
        netdiag->setErrorCode(102052);
    }

    mDiagState = eFinish;

    NETWORK_LOG_INFO("End:%s\n", netdiag->getTestType());
    return 0;
}

int 
NetworkSpeedDiagnose::stop()
{
    NETWORK_LOG_INFO("stop:%d\n", type());
    if (mDiagState == eRun)
        mDiagState = eStop;
    return 0;
}

int 
NetworkSpeedDiagnose::doResult(int result, int size, void* data)
{
    double v = 0.0, sum = 0.0;
    time_t now = time(0);
    int mod = 0, i = 0, n = 0;

    if (now >= mTestET || !size)
        return 0;

    if (mDiagState != eRun)
        return 0;

    mod = ((int)difftime(now, mTestST) + 1) % mSamplePeriod;

    mSampleRecvSize += size;
    if (mRecordPoint != now && !mod) {
        v = mSampleRecvSize / mSamplePeriod;
        n = mSampleSpeed.size();
        if (n) { //drop the first sample
            if (mMaxSpeed <= 0.0 || mMaxSpeed < v)
                mMaxSpeed = v;
            if (mMinSpeed <= 0.0 || mMinSpeed > v)
                mMinSpeed = v;
            sum = v;
            for (i = 1; i < n; ++i)
                sum += mSampleSpeed[i];
            mAverageSpeed = sum / n;
        }
        mSampleSpeed.push_back(v);
        mSampleRecvSize = 0;
        mRecordPoint = now;
        NETWORK_LOG_INFO("MaxSpeed:%.2fMbit/s MinSpeed:%.2fMbit/s AverageSpeed:%.2fMbit/s\n", mMaxSpeed / 1000000, mMinSpeed / 1000000, mAverageSpeed / 1000000);
    } 
    return size;
}

static size_t _WriteCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
    NetworkSpeedDiagnose* diagnose = (NetworkSpeedDiagnose*)data;
    if (diagnose)
         return diagnose->doResult(0, size * nmemb, ptr);
    return 0; //
}
