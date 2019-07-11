
#include "MonitorLogPostFTP.h"
#include <sys/time.h>
#include <time.h>
#include "HybroadService.h"

#define FTP_FILE_PATH "/data/ftpupload.log"
#define FTP_FILE_SIZE  128 * 1024

extern android::HybroadService *pHybroad;
namespace android {

MonitorLogPostFTP::MonitorLogPostFTP(const char* server, const char* userPwd, int logType, int logLevel)
    : mServer(server), mUserPwd(userPwd), mWriteFp(NULL), m_logType(logType), m_logLevel(logLevel)
{
    pthread_mutex_init(&mMutex, NULL);
    mWriteFp = fopen(FTP_FILE_PATH, "w");
}

MonitorLogPostFTP::~MonitorLogPostFTP()
{
    pthread_mutex_destroy(&mMutex);
    if (mWriteFp)
        fclose(mWriteFp);
}


void
MonitorLogPostFTP::upload(void* param)
{
    int   res       = 0;
    void* lCurl     = NULL;
    long  lSize     = 0;
	char *uploadFileName = FTP_FILE_PATH;
	char uploadURL[1024] = {0};
	
	sprintf(uploadURL, "%s/ftpupload.log", mServer.c_str());
	
    printf("ftp upload start\n");
    FILE* lReadFp = fopen(uploadFileName, "r");

    if (!lReadFp) {
        printf("fopen error\n");
        return;
    }
    
	fseek(lReadFp, 0L, SEEK_END);
	lSize = ftell(lReadFp);
	fseek(lReadFp, 0L, SEEK_SET);

    lCurl = curl_easy_init(); /* start a libcurl easy session */
    if (!lCurl) {
        printf("curl init error\n");
        fclose(lReadFp);
        return;
    }
    
    curl_easy_setopt(lCurl, CURLOPT_SSH_AUTH_TYPES, CURLSSH_AUTH_PASSWORD);
    curl_easy_setopt(lCurl, CURLOPT_URL, uploadURL); 
    curl_easy_setopt(lCurl, CURLOPT_CONNECTTIMEOUT, 10); /* connnect time */
    curl_easy_setopt(lCurl, CURLOPT_UPLOAD, 1L); /* enable upload */
    curl_easy_setopt(lCurl, CURLOPT_VERBOSE, 1L); /* debug use */
    curl_easy_setopt(lCurl, CURLOPT_READDATA, lReadFp);
    curl_easy_setopt(lCurl, CURLOPT_INFILESIZE, lSize);
    curl_easy_setopt(lCurl, CURLOPT_FTP_CREATE_MISSING_DIRS, 1); /* auto make directory */
    res = curl_easy_perform(lCurl); /* will block */
    if (res != 0)
        printf("curl failed\n");
    curl_easy_cleanup(lCurl); /* end a libcurl easy session */
    
    fclose(lReadFp);

    printf("ftp Upload End!\n");
    return;
}

void
MonitorLogPostFTP::beginUpload()
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
MonitorLogPostFTP::pushBlock(uint8_t* blockHead, uint32_t blockLength)
{
    pthread_mutex_lock(&mMutex);
    if (mWriteFp) {
		if(checkLog((const char *)blockHead, (const unsigned int)blockLength, m_logType, m_logLevel)){
			//printf("ftp log write\n");
			fwrite(blockHead, 1, blockLength, mWriteFp);
		}
		
        if (ftell(mWriteFp) > FTP_FILE_SIZE) {
            fclose(mWriteFp);
            mWriteFp = NULL;
			printf("ftp log upload\n");
            upload(NULL);
            // OSTimerThread(upload, this); /* 3560E not support */
        }
    }
    pthread_mutex_unlock(&mMutex);
    return false;
}


int
MonitorLogPostFTP::checkLog(const char *input, const int input_len, int log_type, int log_level)
{
	int log_info = 0;
	int error_code = 0;
	int input_type = 0;
	
	if(input_len > 5){
		if(input[0] == '<' && input[4] == '>' ){
			if (log_level == 0)
				return 1;
			log_info = 100 *(input[1] - '0') + 10 * (input[2] - '0') + input[3] - '0';
			error_code = log_info % 8;
			switch(error_code){
				case 3: //error
				case 6: //informational
				case 7://debug
				if(log_level < error_code) return 0;
				break;
				default: //unknown
				return 0;
				break;
			}
			if(log_type != 0){
				input_type = log_info - error_code;
				if(input_type  != log_type * 8) return 0;
			}
			return 1;
		}
	}
	return 0;
}

} // End namespace Hippo
