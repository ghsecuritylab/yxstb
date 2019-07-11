
#include "UpgradeReceiver.h"

#include "UpgradeAssertions.h"
#include "DataSource.h"
#include "RingBuffer.h"
#include "UpgradeSource.h"
#include "UpgradeData.h"
#include "UpgradeManager.h"

#include <string>
#include <string.h>
#include <iostream>
#include <stdlib.h>


#define TEMPLATESIZE 8 * 1024 * 1024


namespace Hippo {

UpgradeReceiver::UpgradeReceiver(UpgradeManager* manager, UpgradeSource* source)
	: m_manager(manager)
	, m_source(source)
	, mReceiveLen(0)
{
    m_data = new UpgradeData(UPGRADE_PATH);

    setBuffer(m_source->getRingBuffer());
    m_source->m_dataSource->attachSink(this);
}

UpgradeReceiver::~UpgradeReceiver()
{
    m_source->m_dataSource->detachSink(this);
}

bool
UpgradeReceiver::start()
{
    mReceiveLen = 0;
    m_data->open('w');
    m_source->m_dataSource->start();
    sendEmptyMessageDelayed(MC_TIMER, 1000);
    return true;
}

bool
UpgradeReceiver::stop()
{
    return false;
}

UpgradeData*
UpgradeReceiver::getUpgradeData()
{
    //UpgradeData* data = m_data;
    //m_data = 0;
    return m_data;
}

bool
UpgradeReceiver::onDataArrive()
{
    return sendEmptyMessage(MC_DataArrive);
}

bool
UpgradeReceiver::onError()
{
    return sendEmptyMessage(MC_Error);
}

bool
UpgradeReceiver::onEnd()
{
    return sendEmptyMessage(MC_End);
}

bool
UpgradeReceiver::onTimer()
{
    int process = 0;

    UpgradeLogDebug("mReceiveLen = %d, m_dataSize = %d\n", mReceiveLen, m_dataSize);
    if (mReceiveLen && m_dataSize > 0) {
        if ((m_source->Type() == UpgradeManager::UMUT_IP_TEMPLATE 
            || m_source->Type() == UpgradeManager::UMUT_DVB_TEMPLATE)
            && m_dataSize > TEMPLATESIZE) {
            m_source->m_dataSource->stop();
            return true;
        }
        if (m_dataSize > 1 * 1024 * 1024)
            process = (mReceiveLen/1000) / (m_dataSize/100000);
        else {
            if (m_dataSize > 0 && m_dataSize/1000 > 0)
                process = (mReceiveLen/10) / (m_dataSize/1000);
        }
        m_manager->sendUpgradeMessage(UpgradeManager::UMIT_PROGRESS, process, 0, 0);
    }
    return sendEmptyMessageDelayed(MC_TIMER, 1000);
}

void
UpgradeReceiver::handleMessage(Message* msg)
{
    switch (msg->what) {
    case MC_DataArrive:
        receiveData();
        break;
    case MC_End:
        receiveEnd();
        break;
    case MC_Error:
        receiveError();
        break;
    case MC_TIMER:
        onTimer();
        break;
    default:
        break;
    }
}

void
UpgradeReceiver::receiveData()
{
    if (m_source->bufferType() == UpgradeSource::USBT_RingBuffer) {
        while (1) {
            uint8_t* bufPointer;
            uint32_t bufLength;

            m_ringBuffer->getReadHead(&bufPointer, &bufLength);
            if (bufLength == 0)
                break;

            m_data->write(bufPointer, bufLength);
            mReceiveLen = mReceiveLen + bufLength;
            m_ringBuffer->submitRead(bufPointer, bufLength);
        }
    }
    else if (m_source->bufferType() == UpgradeSource::USBT_RingBufferFragment) {
        ;
    }
}

void
UpgradeReceiver::receiveError()
{
    UpgradeLogError("receiveError\n");
    m_state = URS_ERROR;
    if (m_data) {
        m_data->close();
        m_data->unlink();
        delete m_data;
    }
    removeMessages(MC_TIMER);
    m_manager->sendEmptyMessageDelayed(UpgradeManager::UMMC_RECEIVE_END, 500);
}

void
UpgradeReceiver::receiveEnd()
{
    UpgradeLogDebug("mReceiveLen: %u\n", mReceiveLen);
    m_state = URS_OK;
    m_data->close();
    removeMessages(MC_TIMER);
    //m_manager->upgradeRoportProgress(100);
    m_manager->sendUpgradeMessage(UpgradeManager::UMIT_PROGRESS, 100, 0, 0);
    if (m_source->m_force) {
        UP_HEADER_INFO_b200 UpgradeHeaderInfo;
        /* If "cat app.bin other.bin > upgrade.bin" */
        if (!getUpgradeVersionFromHead(UPGRADE_PATH, 0, &UpgradeHeaderInfo, "root")) {
            m_source->m_version = UpgradeHeaderInfo.svn_version;
        /* If "cat logo.yuf other.bin > upgrade.bin" */
        } else if (!getUpgradeVersionFromHead(UPGRADE_PATH, 0, &UpgradeHeaderInfo, "config")) {
            m_source->m_logoVersion = UpgradeHeaderInfo.svn_version;
            unsigned int logoFileLength = UpgradeHeaderInfo.datasize + 256;
            if (mReceiveLen > logoFileLength + 10*1024*1024) {
                /* If "cat logo.yuf app.bin > upgrade.bin" */
                if (!getUpgradeVersionFromHead(UPGRADE_PATH, logoFileLength, &UpgradeHeaderInfo, "root")) {
                    m_source->m_version = UpgradeHeaderInfo.svn_version;
                /* If "cat logo.yuf kernel.yuf app.bin > upgrade.bin" */
                } else if (!getUpgradeVersionFromHead(UPGRADE_PATH, logoFileLength, &UpgradeHeaderInfo, "kernel")) {
                    unsigned int kernelFileLength = UpgradeHeaderInfo.datasize + 256;
                    if (mReceiveLen > logoFileLength + kernelFileLength + 10*1024*1024) {
                        if (!getUpgradeVersionFromHead(UPGRADE_PATH, logoFileLength + kernelFileLength, &UpgradeHeaderInfo, "root")) {
                            m_source->m_version = UpgradeHeaderInfo.svn_version;
                        }
                    }
                }
            }
        /* If "cat kernel.yuf other.bin > upgrade.bin" */
        } else if (!getUpgradeVersionFromHead(UPGRADE_PATH, 0, &UpgradeHeaderInfo, "kernel")) {
            unsigned int kernelFileLength = UpgradeHeaderInfo.datasize + 256;
#if defined(Cameroon_v5)
            m_source->m_version = UpgradeHeaderInfo.svn_version;
#endif
            if (mReceiveLen > kernelFileLength + 10*1024*1024) {
                /* If "cat kernel.yuf app.bin > upgrade.bin" */
                if (!getUpgradeVersionFromHead(UPGRADE_PATH, kernelFileLength, &UpgradeHeaderInfo, "root")) {
                    m_source->m_version = UpgradeHeaderInfo.svn_version;
                /* If "cat kernel.yuf logo.yuf app.bin > upgrade.bin" */
                } else if (!getUpgradeVersionFromHead(UPGRADE_PATH, kernelFileLength, &UpgradeHeaderInfo, "config")) {
                    m_source->m_logoVersion = UpgradeHeaderInfo.svn_version;
                    unsigned int logoFileLength = UpgradeHeaderInfo.datasize + 256;
                    if (!getUpgradeVersionFromHead(UPGRADE_PATH, kernelFileLength + logoFileLength, &UpgradeHeaderInfo, "root")) {
                        m_source->m_version = UpgradeHeaderInfo.svn_version;
                    }
                }
            }
        }
        if (m_source->m_version > 0)
            m_source->m_logoVersion = -1;
        UpgradeLogDebug("appVersion[%d], logoVersion[%d], settingVersion[%d]\n", m_source->m_version, m_source->m_logoVersion, m_source->m_settingVersion);
        if ((m_source->m_version <= 0) && (m_source->m_logoVersion <= 0) && (m_source->m_settingVersion <= 0))
            getSettingVersion(UPGRADE_PATH);
    }

    m_manager->sendEmptyMessage(UpgradeManager::UMMC_RECEIVE_END);
}

static int yx_read_image_header(char *filename, long offset, UP_HEADER_INFO_b200 *UpgradeHeaderInfo)
{
    if(filename == NULL)
        return -1;

    int ret = 0;
    UP_HEADER_INFO_b200 info;

    memset(&info, 0, sizeof(UP_HEADER_INFO_b200));
    FILE *fp = fopen(filename, "r");
    if(fp == NULL) {
        printf("error! fopen the file  %s is error\n", filename);
        return -1;
    }

    fseek(fp, offset, SEEK_SET);
    ret = fread(&info, sizeof(UP_HEADER_INFO_b200), 1, fp);
    fclose(fp);
    if(ret == 0)
        return -1;
    memcpy(UpgradeHeaderInfo, &info, sizeof(UP_HEADER_INFO_b200));
    return 0;
}

int UpgradeReceiver::getUpgradeVersionFromHead(char *path, long offset, UP_HEADER_INFO_b200 *headInfo, const char *targetName)
{
    memset(headInfo, 0, sizeof(UP_HEADER_INFO_b200));
    if (!yx_read_image_header(path, offset, headInfo)) {
        if (!strncmp(headInfo->target, targetName, strlen(targetName))) {
            UpgradeLogDebug("Read: target[%s], version[%d]\n", headInfo->target, headInfo->svn_version);
            return 0;
        } else {
            return -1;
        }
    } else {
        return -1;
    }
}

void UpgradeReceiver::getSettingVersion(const char *path)
{
    using namespace std;
    string versionLine;
    string version;
    int result0 = -1, result1 = -1;
    FILE* fp = fopen(UPGRADE_PATH, "rb");
    if (!fp) {
        UpgradeLogError("Open setting upgrade file failed!\n");
        return;
    }
    char    buffer[40960];
    if (fgets(buffer, sizeof(buffer), fp)) {
        while(buffer[strlen(buffer) - 1] == '\r' || buffer[strlen(buffer) - 1] == '\n')
            buffer[strlen(buffer) - 1] = '\0';
        versionLine = buffer;
        result0  = versionLine.find("settingVersion", 0);
        if (-1 != result0) {
            result0 = versionLine.find('=', 13);
            if (-1 != result0) {
                result1 = versionLine.find_last_of('.');
                if (-1 != result1) {
                    version = versionLine.substr(result1 + 1);
                } else {
                    result1 = versionLine.find_last_of(' ');
                    if (-1 != result1)
                        version = versionLine.substr(result1 + 1);
                    else
                        version = versionLine.substr(result0 + 1);
                }
                m_source->m_settingVersion = atoi(version.c_str());
                UpgradeLogDebug("Read setting version: %d\n", m_source->m_settingVersion);
                fclose(fp);
                return;
            }
        }
    }
    fclose(fp);
    UpgradeLogError("Read setting verion failed! Right format: settingVersion = xxxx\n");
}

} // namespace Hippo

