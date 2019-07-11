#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "RegularFileInfo.h"
#include "Assertions.h"

#include "SystemManager.h"
#include "UltraPlayer.h"

namespace Hippo{

RegularFileInfo::RegularFileInfo(std::string path)
:FileInfo(path, Regular_file)
,m_classification(RegularFileInfo::UnkonwFile)
,m_isInfoSync(false)
,m_artist("unknow")
,m_album("unknow")
,m_year("unknow")
,m_comments("unknow")
,m_thumbnailPath("")
{
    m_classification = getFileclassification(m_name); 
    if (m_classification == ImageFile) {
        //m_thumbnailPath = m_fullPath;    
        unsigned pos = m_fullPath.find_last_of("/");
        std::string thumbnailPath = m_fullPath.substr(0, pos) + "/.thumbnail/" + m_name;
        if (!access(thumbnailPath.c_str(), 0)) {
             m_thumbnailPath = thumbnailPath;  
        } 
    }       
}    

RegularFileInfo::~RegularFileInfo()
{
    
}

int RegularFileInfo::getTotalTime()
{
    SystemManager &sysManager = systemManager();
    UltraPlayer *player = sysManager.obtainMainPlayer();
    int Duration = 0;

    if(player) {
        Duration = player->getTotalTime();
    }
    sysManager.releaseMainPlayer(player);
    return Duration;
}
std::string RegularFileInfo::getAudioLanguage(int index)
{
    if ((index < 0) || index >= m_audioCount){
        perror("input index error\n");
        return "";    
    }    
    return m_audioLanguage[index];    
}

int RegularFileInfo::setAudioLanguage(int index, const char* lan)
{
    if ((index < 0) || index >= m_audioCount){
        perror("input index error\n");
        return -1;    
    }
    m_audioLanguage[index] = lan;
    return 0;    
}

std::string RegularFileInfo::getAudioFormate(int index)
{
    if ((index < 0) || index >= m_audioCount){
        perror("input index error\n");
        return "";    
    }    
    return  m_audioFormate[index];     
}

int RegularFileInfo::setAudioFormate(int index, const char* formate)
{
    if ((index < 0) || index >= m_audioCount){
        perror("input index error\n");
        return -1;    
    }
    m_audioFormate[index] = formate;
    return 0;      
}
    
int RegularFileInfo::getAudioTotalTime(int index)
{
    if ((index < 0) || index >= m_audioCount){
        perror("input index error\n");
        return 0;    
    }    
    
    return m_audioTotalTime[index];    
}

int RegularFileInfo::setAudioTotalTime(int index, int t)
{
    if ((index < 0) || index >= m_audioCount){
        perror("input index error\n");
        return -1;    
    }
    m_audioTotalTime[index] = t;
    return 0;    
}
    
int RegularFileInfo::getAudioSampleRate(int index)
{
    if ((index < 0) || index >= m_audioCount){
        perror("input index error\n");
        return 0;    
    }    
    
    return m_sampleRate[index];    
}


int RegularFileInfo::setAudioSampleRate(int index, int sampleRate)
{
    if ((index < 0) || index >= m_audioCount){
        perror("input index error\n");
        return -1;    
    }
    m_sampleRate[index] = sampleRate;
    return 0;     
}
    
int RegularFileInfo::getAudioBitRate(int index)
{
    if ((index < 0) || index >= m_audioCount){
        perror("input index error\n");
        return 0;    
    }    
    
    return m_audioBitRate[index];    
}    


int RegularFileInfo::setAudioBitRate(int index, int bitRate)
{
    if ((index < 0) || index >= m_audioCount){
        perror("input index error\n");
        return -1;    
    }
    m_audioBitRate[index] = bitRate;
    return 0;        
}    

RegularFileInfo::FileClassification RegularFileInfo::getFileclassification(std::string name)
{
    const char *videoFormatList[] = {"mpg", "mpa", "mpeg", "mp4", "mkv", "ts", "avi", "asf", "vob", "mov", "m2ts", "mts", "dat", "f4v", "flv", "wmv"};
    const char *audioFormatList[] = {"mp3", "wav", "wma", "m4a", "aac"};
    const char *imageFormateList[] = {"jpg", "bmp", "png", "gif", "tiff", "jpeg"};
    
    int videoFormatCount = sizeof(videoFormatList)/sizeof(videoFormatList[0]);   
    int audioFormatCount = sizeof(audioFormatList)/sizeof(audioFormatList[0]);
    int imageFormateCount = sizeof(imageFormateList)/sizeof(imageFormateList[0]);
    
    unsigned pos = name.find_last_of(".");
    if (pos == std::string::npos) {
        return RegularFileInfo::UnkonwFile;
    }
    if (pos + 1 >= name.length()) {
        return RegularFileInfo::UnkonwFile;    
    }
    std::string extension = name.substr(pos + 1);
    for(int i = 0; i < imageFormateCount; i++) {
        if (!strcasecmp(extension.c_str(), imageFormateList[i])) {
            return  RegularFileInfo::ImageFile;   
        }
    }        
      
    for(int i = 0; i < audioFormatCount; i++) {
        if (!strcasecmp(extension.c_str(), audioFormatList[i])) {
            return  RegularFileInfo::AudioFile;   
        }
    }
            
    for(int i = 0; i < videoFormatCount; i++) {
        if (!strcasecmp(extension.c_str(), videoFormatList[i])) {
            return  RegularFileInfo::VideoFile;   
        }
    } 
    
    return RegularFileInfo::UnkonwFile;             
}    

int RegularFileInfo::removeFile()
{
    LogUserOperDebug("remove rugularfile [%s]\n", m_fullPath.c_str());
    unlink(m_fullPath.c_str());
    return 0;
}

}; //namespace Hippo