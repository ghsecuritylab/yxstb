/************************************************************
Copyright: 2010-2013 Hybroad Vision Corporation All Rights Reserved.
Author: zhangming
Date: 2013-9-24 10:01
Description: Video file parser base class
************************************************************/
#ifndef VideoFileParser_h
#define VideoFileParser_h

#include <string>
#include <vector>
#define UINT64_C long
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"

namespace Hippo {
   
class VideoFileParser {
public:  
    VideoFileParser(const char* path);
    ~VideoFileParser();
    
    virtual int extractPosterImage(const char*);
    
protected:
    std::string m_path;   
    long m_size;  //byte
    int m_duration; // s
    int m_bitRate; // kb/s
    std::string m_videoFormate;
    int m_frameRate;
    int m_with;
    int m_height;
    std::vector<std::string> m_audioFormate;
    std::vector<std::string> m_audioLanguage;
    std::vector<std::string> m_audioSoundTrack;
    std::vector<int> m_audioBitrate;    
    std::vector<int> m_sampleRate;
    int SaveFrame(AVFrame *pFrame, int width, int height, int iFrame, const char* filePath);
    int writeBMPhead(int nWidth,int nHeight,FILE*fp1);
        

}; //class VideoFileParser
    
}; //namespace Hippo

#endif //VideoFileParser_h

