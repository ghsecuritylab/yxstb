/************************************************************
Copyright: 2010-2013 Hybroad Vision Corporation All Rights Reserved.
Author: zhangming
Date: 2013-9-24 10:05
Description: mp3 file parser  class
************************************************************/
#include "Assertions.h"
#include "Mp3FileParser.h"
#include "CodeTransform.h"
#include <unistd.h>
#include <stdio.h>

namespace Hippo {

Mp3FileParser::Mp3FileParser(const char* path)
:AudioFileParser(path)
{
    if (!path) {
        LogUserOperError("Error, Mp3FileParser path is NULL\n");
        return;    
    }
    if (access(path, F_OK)) { 
        LogUserOperError("Mp3FileParser error, file not exist\n");
        return;    
    }
    FILE* fp = fopen(path, "r");
    if (!fp) {
        LogUserOperError("Mp3FileParser error, fopen error\n"); 
        return ;   
    }
    if (fseek(fp, -128, SEEK_END)) {
        LogUserOperError("Mp3FileParser error, fseek error\n");   
        fclose(fp);
        return;  
    }
    char buf[1024] = {0};
    if (fread(buf, 1, 128, fp) != 128) {
        LogUserOperError("Mp3FileParser error, fread error\n");   
        fclose(fp);
        return;          
    }
    if (strncmp(buf, "TAG", 3)) {
        //LogUserOperError("Mp3FileParser error, not find TAG string\n"); 
        fclose(fp);
        return;
    }
    m_size = ftell(fp) + 128;
    fclose(fp);
    char *ptr = buf;
    char tempBuf[64] = {0};
    char outBuf[128] = {0};
    int outLen = 0;
    
    memset(tempBuf, 0, sizeof(tempBuf));
    ptr += 3;
    memcpy(tempBuf, ptr, 30);
    m_title = tempBuf;
        
    memset(tempBuf, 0, sizeof(tempBuf));
    ptr += 30;
    memcpy(tempBuf, ptr, 30);
    memset(outBuf, 0, sizeof(outBuf));
    GBtoUtf8((unsigned char*)tempBuf, 30,(unsigned char*)outBuf, &outLen);
    m_artist = outBuf;
        
    memset(tempBuf, 0, sizeof(tempBuf));
    ptr += 30;
    memcpy(tempBuf, ptr, 30);    
    memset(outBuf, 0, sizeof(outBuf));
    GBtoUtf8((unsigned char*)tempBuf, 30, (unsigned char*)outBuf, &outLen);
    m_album = outBuf;    
        
    memset(tempBuf, 0, sizeof(tempBuf));
    ptr += 30;
    memcpy(tempBuf, ptr, 4);    
    m_year = tempBuf;    

    memset(tempBuf, 0, sizeof(tempBuf));
    ptr += 4;
    memcpy(tempBuf, ptr, 28);    
    m_comments = tempBuf;

    memset(tempBuf, 0, sizeof(tempBuf));
    ptr += 30;
    memcpy(tempBuf, ptr, 1);    
    std::string m_Genre = tempBuf;

#if 0    
	static AVFormatContext *pFormatCtx;
	av_register_all();
    if (avformat_open_input(&pFormatCtx, path, NULL, NULL) < 0) {
        LogUserOperError("Cannot open input file\n");
        return;
    }
	// Find the first video stream
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        LogUserOperError( "Cannot find stream information\n");
        return;
    }	
    m_duration = (pFormatCtx->duration + 5000) / AV_TIME_BASE;
    m_bitRate = pFormatCtx->bit_rate / 1000;  
    av_close_input_file(pFormatCtx);
#endif    
}        

int Mp3FileParser::extractAlbumThumbnail(const char* savePath)
{
    std::string savePicPath = "";
    if (savePath) 
        savePicPath = savePath;
    else
        savePicPath = m_path;
    savePicPath = savePicPath + ".jpg";     
    LogUserOperDebug("extractAlbumThumbnail path is [%s]\n", savePicPath.c_str());
    if (!access(savePicPath.c_str(), F_OK)) {
        LogUserOperDebug("extractAlbumThumbnail file [%s] already exist\n", savePicPath.c_str());
        return 0;    
    }    
    if (access(m_path.c_str(), F_OK)) { 
        LogUserOperError("extractAlbumThumbnail error, file not exist\n");
        return -1;    
    }   
    FILE* fp = fopen(m_path.c_str(), "r");
    if (!fp) {
        LogUserOperError("extractAlbumThumbnail error, open file error\n");
        return -1;    
    } 
    char ID3headBuf[16] = {0};
    int ret = fread(ID3headBuf, 1, 10, fp);
    if (ret != 10) {
        LogUserOperError("fread ID3head error\n");
        fclose(fp);
        return -1;    
    }
    if (strncmp(ID3headBuf, "ID3", 3)) {
        LogUserOperError("extractAlbumThumbnail error, not find ID3 tag\n");
        fclose(fp);
        return -1;    
    }
    int ID3V2FrameSize = (int)(ID3headBuf[6] & 0x7F) << 21
                        | (int)(ID3headBuf[7] & 0x7F) << 14
                        | (int)(ID3headBuf[8] & 0x7F) << 7
                        | (int)(ID3headBuf[9] & 0x7F);
    if (ID3V2FrameSize > 1024 * 1024) {
        LogUserOperError("ID3V2FrameSize > 1024*1024 error\n");
        fclose(fp);
        return -1;     
    }             
    char* frameBuf = (char*)malloc(ID3V2FrameSize + 1);
    if (!frameBuf) {
        LogUserOperError("malloc frameBuf error\n");
        fclose(fp);
        return -1;    
    }      
    ret = fread(frameBuf, 1, ID3V2FrameSize, fp); 
    if (ret != ID3V2FrameSize) {
        LogUserOperError("fread frame data error\n");    
        fclose(fp);
        return -1;
    } 
    fclose(fp);  
    int index = 0;
    int size = 0;
    while(index < ID3V2FrameSize) {
       if (!strncmp(&(frameBuf[index]), "APIC", 4)) {
            int descTestLen = 0;
            index = index + 4;
            
            size = (((int)frameBuf[index] & 0xff) << 24) 
                    | (((int)frameBuf[index] & 0xff) << 16) 
                    | (((int)frameBuf[index + 2] & 0xff) << 8) 
                    | (frameBuf[index + 3] & 0xff);
            index = index + 6;
            char textEncode = frameBuf[index];
            int endTagLen = 1;
            if (textEncode == 0x01) 
                endTagLen = 2;
            index = index + 1;
            descTestLen = 1;
                            
            #define MaxStringLen 64    
            char MIMEtype[MaxStringLen] = {0};

            int i = 0;
            for (i = 0; i < MaxStringLen; i++) {
                if (frameBuf[index + i] == 0) {
                    break;
                 }   
                 MIMEtype[i] = frameBuf[index + i];
            }
            if (i >= MaxStringLen) {
                LogUserOperError("mime type len error\n");
                return -1;    
            }
            index = index + i + endTagLen;
            descTestLen = descTestLen + i + endTagLen;
            //char picType =  frameBuf[index];
            index++;
             descTestLen = descTestLen + 1;
            char description[MaxStringLen] = {0};
            for (i = 0; i < MaxStringLen; i++) {
                if (frameBuf[index + i] == 0) {
                    break;
                 }   
                 description[i] = frameBuf[index + i];
            } 
            if (i >= MaxStringLen) {
                LogUserOperError("mime type len error\n");
                return -1;    
            } 
            index = index + i + endTagLen;
            descTestLen = descTestLen + i + endTagLen;                      
            FILE* picFp = fopen(savePicPath.c_str(), "w+"); 
            if (!picFp) {
                LogUserOperError("fopen error\n");
                return -1;    
            }
            fwrite(&(frameBuf[index]), 1, (size - descTestLen), picFp);
            fclose(picFp);
            
            break;   
        } 
        else {   
            index = index + 4;
            size = (((int)frameBuf[index] & 0xff) << 24) 
                    | (((int)frameBuf[index] & 0xff) << 16) 
                    | (((int)frameBuf[index + 2] & 0xff) << 8) 
                    | (frameBuf[index + 3] & 0xff);
            index = index + size + 4 + 2;
        }
    } 
    
    return 0;    
}

}; //namespace Hippo