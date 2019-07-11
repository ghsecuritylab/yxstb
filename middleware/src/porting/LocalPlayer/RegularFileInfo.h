#ifndef RegularFileInfo_H
#define RegularFileInfo_H

#include <unistd.h>
#include <string>
#include "FileInfo.h"

#define MAX_AUDIO_COUNT 6

namespace Hippo{
    
class RegularFileInfo : public FileInfo {
public:
    enum FileClassification{
        UnkonwFile,
        VideoFile,
        AudioFile,
        ImageFile, 
    };
    RegularFileInfo(std::string path);
    ~RegularFileInfo();
    FileClassification getClassification(){ return m_classification; }
    virtual int removeFile();
    int setThumbnailPath(std::string path) {m_thumbnailPath = path; return 0;}
    std::string getThumbnailPath(){ return m_thumbnailPath; }  
    bool isInfoSync() { return m_isInfoSync;}
    int setInfoSync(bool flag) {m_isInfoSync = flag; return 0;}
    int getFileTotalTime(){ return m_fileTotalTime;}
    int setFileTotalTime(int t){m_fileTotalTime = t; return 0;}
    int getFileBitRate(){return m_videoBitRate;}
    int setFileBitRate(int bitRate){m_videoBitRate = bitRate; return 0;}
    std::string getVideoFormate() {return m_videoFormate;}
    int setVideoFormate(const char* format){m_videoFormate = format; return 0;}
    int getVideoTotalTime(){ return m_videoTotalTime;}   
    int setVideoTotalTime(int t){m_videoTotalTime = t; return 0;}
    int getVideoBitRate(){return m_videoBitRate;} 
    int setVideoBitRate(int bitRate){m_videoBitRate = bitRate; return 0;}
    int getVideoWidth(){return m_videoWidth;}
    int setVideoWidth(int w){m_videoWidth = w; return 0;}
    int getVideoHeight(){return m_videoHeight;}
    int setVideoHeight(int h){m_videoHeight = h; return 0;}
    std::string getFrameRate(){return m_frameRate;}
    int setFrameRate(const char* frameRate){m_frameRate = frameRate; return 0;}
    int getAudioCount(){return m_audioCount;}
    int setAudioCount(int count){m_audioCount = count; return 0;}
    std::string getAudioLanguage(int);
    int setAudioLanguage(int, const char*);
    std::string getAudioFormate(int);
    int setAudioFormate(int, const char*);
    int getAudioTotalTime(int);
    int setAudioTotalTime(int, int);
    int getAudioSampleRate(int);
    int setAudioSampleRate(int, int);
    int getAudioBitRate(int);
    int setAudioBitRate(int, int);
    int getTotalTime();
    int setArtist(std::string artist) { m_artist = artist; return 0;}
    std::string getArtist() { return m_artist;}           
    int setAlbum(std::string album) { m_album = album; return 0;}    
    std::string getAlbum() { return m_album;}    
    int setYear(std::string year) {m_year = year; return 0;}
    std::string getYear() {return m_year;}
    int setComments(std::string comments) {m_comments = comments; return 0;}
    std::string getComments() {return m_comments;}
    static FileClassification getFileclassification(std::string name);    
    
protected:
    FileClassification m_classification;
    bool m_isInfoSync;
    int m_fileTotalTime; // s
    int m_fileBitRate;
    std::string m_videoFormate;
    int m_videoTotalTime; // s
    int m_videoBitRate;
    int m_videoWidth;
    int m_videoHeight;
    std::string m_frameRate;
    int m_audioCount;
    std::string m_audioLanguage[MAX_AUDIO_COUNT];
    std::string m_audioFormate[MAX_AUDIO_COUNT]; 
    int m_audioTotalTime[MAX_AUDIO_COUNT]; // s
    int m_sampleRate[MAX_AUDIO_COUNT];  
    int m_audioBitRate[MAX_AUDIO_COUNT];  
    std::string m_artist; 
    std::string m_album;    
    std::string m_year;
    std::string m_comments;
    std::string m_thumbnailPath;
}; //class RegularFileInfo
    
}; //namespace Hippo

#endif //RegularFileInfo_H
 