/************************************************************
Copyright: 2010-2013 Hybroad Vision Corporation All Rights Reserved.
Author: zhangming
Date: 2013-9-23 16:05
Description: file parser base class
************************************************************/
#ifndef AudioFileParser_h
#define AudioFileParser_h

#include <string>

namespace Hippo {

class AudioFileParser {
public:    
    AudioFileParser(const char* path)
    :m_size(0)
    ,m_duration(0)
    ,m_title("unknow")
    ,m_artist("unknow")
    ,m_album("unknow")
    ,m_year("unknow")
    ,m_comments("unknow")
    ,m_Genre("unknow")
    ,m_bitRate(0)
    {
        m_path = path;
    }
    ~AudioFileParser(){}
    
    int getLength();
    int getSize();
    std::string getArtist(){ return m_artist;}
    std::string getAlbum(){ return m_album;}
    std::string getYear() {return m_year;}
    std::string getComments(){return m_comments;}
    std::string getGenre(){return m_Genre;}
    int getBitRate();
    
    virtual int extractAlbumThumbnail(const char*) = 0;
    
protected:
    std::string m_path;   
    long m_size;  //byte
    int m_duration; // s
    std::string m_title;
    std::string m_artist; 
    std::string m_album; 
    std::string m_year;
    std::string m_comments;    
    std::string m_Genre; 
    int m_bitRate; // kb/s
}; //class FilePaser
    
}; //namespace Hippo

#endif //AudioFileParser_h