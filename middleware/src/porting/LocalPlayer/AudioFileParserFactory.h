/************************************************************
Copyright: 2010-2013 Hybroad Vision Corporation All Rights Reserved.
Author: zhangming
Date: 13/11/5 9:37
Description: audioFileParserFactory
************************************************************/
#include "AudioFileParser.h"
#include <string>

namespace Hippo {

class AudioFileParserFactory {
public:
    AudioFileParserFactory();
    ~AudioFileParserFactory();
    
    int setPath(std::string path);
    AudioFileParser* getParser();
        
private:       
    std::string m_path; 
}; //class AudioFileParserFactory  

}; //namespace Hippo
