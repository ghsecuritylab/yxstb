/************************************************************
Copyright: 2010-2013 Hybroad Vision Corporation All Rights Reserved.
Author: zhangming
Date: 2013-9-24 10:05
Description: mp3 file parser  class
************************************************************/
#ifndef Mp3FileParser_h
#define Mp3FileParser_h

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "AudioFileParser.h"
//#define UINT64_C long
//#include "libavcodec/avcodec.h"
//#include "libavformat/avformat.h"
namespace Hippo {

class Mp3FileParser : public AudioFileParser {
public:
    Mp3FileParser(const char*);
    ~Mp3FileParser();
    
    virtual int extractAlbumThumbnail(const char*);     
}; //

}; //namespace Hippo

#endif //Mp3FileParser_h
