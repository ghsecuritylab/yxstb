/************************************************************
Copyright: 2010-2013 Hybroad Vision Corporation All Rights Reserved.
Author: zhangming
Date: 13/11/5 9:37
Description: audioFileParserFactory
************************************************************/
#include "Mp3FileParser.h"
#include "AudioFileParserFactory.h"
#include <unistd.h>
#include <ctype.h>
#include <string>
#include <algorithm>

namespace Hippo {

AudioFileParserFactory::AudioFileParserFactory()
{
}    

AudioFileParserFactory::~AudioFileParserFactory()
{
    
}

int AudioFileParserFactory::setPath(std::string path)
{
    m_path = path;    
    return 0;
}    

AudioFileParser* AudioFileParserFactory::getParser()
{
    AudioFileParser* parser = 0;
    std::string subfix = m_path.substr(m_path.rfind(".") + 1);   
    printf("subfix [%s]\n", subfix.c_str());
    std::transform( subfix.begin(), subfix.end(), subfix.begin(), std::ptr_fun <int, int> ( std::tolower ) );  
    if (!subfix.compare("mp3"))  
        parser = new Mp3FileParser(m_path.c_str());
    else if (!subfix.compare("wma")) {
        //do something    
    }
    
    return parser;
}
        
}; //namespace Hippo
