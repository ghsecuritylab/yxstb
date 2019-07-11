
#include "LogPostFTP.h"
#include "SysTime.h"
#include "config/pathConfig.h"
#include "mid_task.h"

#define FTP_FILE_PATH DEFAULT_TEMP_DATAPATH"/ftpupload.log"
#define FTP_FILE_SIZE 131072  // 128 * 1024

#include "NetworkFunctions.h"

namespace Hippo {

LogPostFTP::LogPostFTP(const char* server, const char* userPwd)
    : mServer(server), mUserPwd(userPwd), mWriteFp(NULL)
{
    pthread_mutex_init(&mMutex, NULL);
    mWriteFp = fopen(FTP_FILE_PATH, "w");
}

LogPostFTP::~LogPostFTP()
{
    pthread_mutex_destroy(&mMutex);
    if (mWriteFp)
        fclose(mWriteFp);
}

void
LogPostFTP::upload(void* param)
{
    printf("Ftp Upload Start!\n");
    if (!param)
        return;
    LogPostFTP* self = (LogPostFTP*)param;

    CURLcode res= CURLE_OK;
    CURL* lCurl = NULL;
    long  lSize = 0;
    char  lMac[64] = { 0 };
    char  lFtpUrl[256] = { 0 };
    FILE* lReadFp = fopen(FTP_FILE_PATH, "r");
    if (!lReadFp) {
        self->mWriteFp = fopen(FTP_FILE_PATH, "w");
        return ;
    }
	fseek(lReadFp, 0L, SEEK_END);
	lSize = ftell(lReadFp);
	fseek(lReadFp, 0L, SEEK_SET);

    network_tokenmac_get(lMac, 64, '-');
    SysTime::DateTime lDt;
    SysTime::GetDateTime(&lDt);
    snprintf(lFtpUrl, 255, "%s/%s_%04d%02d%02d%02d%02d%02d.log", self->mServer.c_str(),
        lMac, lDt.mYear, lDt.mMonth, lDt.mDay, lDt.mHour, lDt.mMinute, lDt.mSecond);

    //curl_global_init(CURL_GLOBAL_ALL); /* sets up the program environment that libcurl needs */
    lCurl = curl_easy_init(); /* start a libcurl easy session */
    if (!lCurl)
        goto End;
    curl_easy_setopt(lCurl, CURLOPT_SSH_AUTH_TYPES, CURLSSH_AUTH_PASSWORD);
    curl_easy_setopt(lCurl, CURLOPT_URL, lFtpUrl); /* eg. ftp://user:password@ftp.example.com/upload.log */
    if (!self->mUserPwd.empty())
        curl_easy_setopt(lCurl, CURLOPT_USERPWD, self->mUserPwd.c_str()); /* eg. user:password */
    curl_easy_setopt(lCurl, CURLOPT_TIMEOUT, 3); /* transmission time */
    curl_easy_setopt(lCurl, CURLOPT_CONNECTTIMEOUT, 3); /* connnect time */
    curl_easy_setopt(lCurl, CURLOPT_UPLOAD, 1L); /* enable upload */
    curl_easy_setopt(lCurl, CURLOPT_VERBOSE, 1L); /* debug use */
    curl_easy_setopt(lCurl, CURLOPT_READDATA, lReadFp);
    curl_easy_setopt(lCurl, CURLOPT_INFILESIZE, lSize);
    curl_easy_setopt(lCurl, CURLOPT_FTP_CREATE_MISSING_DIRS, 1); /* auto make directory */
    for (int i = 0; i < 3; ++i) {
        res = curl_easy_perform(lCurl); /* will block */
        if (res == CURLE_OK)
            break;
        printf("failed: %s\n", curl_easy_strerror(res));
    }
    curl_easy_cleanup(lCurl); /* end a libcurl easy session */
    //curl_global_cleanup(); /* release resources acquired by curl_global_init */
End:
    fclose(lReadFp);
    /* reopen the ftp log file */
    self->mWriteFp = fopen(FTP_FILE_PATH, "w");
    printf("Ftp Upload End!\n");
}

void
LogPostFTP::beginUpload()
{
    pthread_mutex_lock(&mMutex);
    if (mWriteFp) {
        fclose(mWriteFp);
        mWriteFp = NULL;
        upload(this); // will be block 3s
    }
    pthread_mutex_unlock(&mMutex);
}

bool
LogPostFTP::pushBlock(uint8_t* blockHead, uint32_t blockLength)
{
    pthread_mutex_lock(&mMutex);
    if (mWriteFp) {
        fwrite(blockHead, 1, blockLength, mWriteFp);
        if (ftell(mWriteFp) > FTP_FILE_SIZE) {
            fclose(mWriteFp);
            mWriteFp = NULL;
            mid_task_create("stb_syslog_output", upload, this);
            // OSTimerThread(upload, this); /* 3560E not support */
        }
    }
    pthread_mutex_unlock(&mMutex);
    return false;
}

} // End namespace Hippo
