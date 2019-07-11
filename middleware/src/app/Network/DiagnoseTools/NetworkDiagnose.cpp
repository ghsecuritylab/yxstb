#include "NetworkAssertions.h"
#include "NetworkDiagnose.h"

#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h> 

NetworkDiagnose::NetworkDiagnose()
    : mTestState(ND_STATE_RUNNING)
    , mTestResult(0)
    , mErrorCode(0)
    , mIsRun(0)
    , mDiagnose(0)
{
    start();
    pipe(mPipeFds);
}

NetworkDiagnose::~NetworkDiagnose()
{
    int on = 0;
    testStop(); 
    write(mPipeFds[1], &on, sizeof(on));
    while (mIsRun)
        usleep(200000);
    if (mDiagnose)
        delete mDiagnose;
}

int 
NetworkDiagnose::testStart(DiagnoseProcess* diagnose)
{
    int on = 1;
    if (diagnose) {
        if (mDiagnose) {
            mDiagnose->stop();
            delete mDiagnose;
        }
        mDiagnose = diagnose;
        mTestState = ND_STATE_RUNNING;
        mErrorCode = 0;
        write(mPipeFds[1], &on, sizeof(on));
    }
    return 0;
}

int 
NetworkDiagnose::testStop()
{
    if (mDiagnose)
        mDiagnose->stop();
    return 0;
}

void 
NetworkDiagnose::run() //This is a thread.
{
    mIsRun = 1;

    int retval = -1;
    for(;;) {
        read(mPipeFds[0], &retval, sizeof(retval));
        if (retval == 0)
            break;
        if (mDiagnose)
            mDiagnose->start(this); //will block
    }

    mIsRun = 0;
}

static NetworkDiagnose* gNetDiag = 0;
NetworkDiagnose* networkDiagnose() 
{
    if (!gNetDiag)
        gNetDiag = new NetworkDiagnose();
    return gNetDiag;
}

char* SplitUrl2Host(const char* u, char* s, int len, int* p)
{
    char port[8] = { 0 };
    char *r = 0, *t = 0;

    if (!u || !s)
        return 0;

    r = strstr((char*)u, "://"); //strip: http:// or https://
    if (!r)
        r = (char*)u;
    else 
        r = r + strlen("://");

    t = strchr(r, ':'); //style: http://ip:port/index.html
    if (t) { 
        //ip or domain
        strncpy(s, r, (t-r) > len ? len : (t-r));
        //port
        if (p) {
            r = port;
            while (isdigit(*(++t))) 
                *r++ = *t;
            *p = atoi(port); 
        }
    } else {
        //ip or domain
        t = strchr(r, '/'); //style: http://host/index.html
        if (t) 
            strncpy(s, r, (t-r) > len ? len : (t-r));
        else 
            strncpy(s, r, len);
    }
    return t; //remain string: /index.html
}
