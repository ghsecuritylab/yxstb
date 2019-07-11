/************************************************************
Copyright: 2010-2013 Hybroad Vision Corporation All Rights Reserved.
Author: zhangming
Date: 2013-9-24 10:01
Description: Video file parser base class
************************************************************/
#include "VideoFileParser.h"
#include "bmpHeadInfo.h"
#include <stdio.h>
#include <string.h>



namespace Hippo {
    
VideoFileParser::VideoFileParser(const char* path)
:m_size(0)
,m_duration(0)
,m_bitRate(0)
,m_videoFormate("")
,m_frameRate(0)
,m_with(0)
,m_height(0)
{
    if (!path) {
        perror("VideoFileParser error, path is NULL \n");
        return;    
    }   
         
    static AVFormatContext *pFormatCtx;
	//static AVCodecContext *pCodecCtx;
	//AVCodec *pCodec;
	//AVFrame *pFrame;
	//AVFrame *pFrameRGB;
	//AVPacket packet;
	//int frameFinished;
	//int numBytes;
	//uint8_t *buffer;
	// Register all formats and codecs
	av_register_all();
    
   // int ret = 0;
    if (avformat_open_input(&pFormatCtx, path, NULL, NULL) < 0) {
        perror("Cannot open input file\n");
        return ;
    }
	// Find the first video stream
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        perror( "Cannot find stream information\n");
        return ;
    }
    
    m_duration = (pFormatCtx->duration + 5000) / AV_TIME_BASE;
    m_bitRate = pFormatCtx->bit_rate / 1000;   

    for(unsigned int i=0;i<pFormatCtx->nb_streams;i++)
    {
       AVStream *st = pFormatCtx->streams[i];
       AVCodecContext* enc = st->codec;
       AVDictionaryEntry *lang = av_dict_get(st->metadata, "language", NULL, 0);
       //const char* codec_type = av_get_media_type_string(enc->codec_type);
       const char* codec_name = avcodec_get_name(enc->codec_id);   
       switch (enc->codec_type) {
       case AVMEDIA_TYPE_VIDEO:
            m_videoFormate = codec_name;
            if (enc->width) {
                m_with = enc->width;
                m_height = enc->height;   
            }
            break;
       case AVMEDIA_TYPE_AUDIO:
            m_audioFormate.push_back(codec_name);
            if (enc->sample_rate) 
                m_sampleRate.push_back(enc->sample_rate);
            else
                m_sampleRate.push_back(0);
            if (lang)
                m_audioLanguage.push_back(lang->value);
            else
                m_audioLanguage.push_back("");    
            break;
       default:
            break;  
       }     
    }  
}    

VideoFileParser::~VideoFileParser()
{
}

int VideoFileParser::extractPosterImage(const char* path)
{
	static AVFormatContext *pFormatCtx;
	int videoStream;
	static AVCodecContext *pCodecCtx;
	AVCodec *pCodec;
	AVFrame *pFrame;
	AVFrame *pFrameRGB;
	AVPacket packet;
	int frameFinished;
	int numBytes;
	uint8_t *buffer;
	
	std::string saveImagePath;
	if (!path)
	    saveImagePath = m_path;
    else
        saveImagePath = path;
    saveImagePath = saveImagePath + ".bmp"  ;
    // Register all formats and codecs  	    
	av_register_all();
    
    int ret = 0;
    if ((ret = avformat_open_input(&pFormatCtx, m_path.c_str(), NULL, NULL)) < 0) {
        perror("Cannot open input file\n");
        return ret;
    }
	// Find the first video stream
	videoStream = -1;
    if ((ret = avformat_find_stream_info(pFormatCtx, NULL)) < 0) {
        perror( "Cannot find stream information\n");
        return ret;
    }
    
    videoStream = ret;	
	if (videoStream == -1) {
	    perror("Error, videoStream is -1\n");
		return -1; // Didn't find a video stream
	}
	// Get a pointer to the codec context for the video stream
	pCodecCtx = pFormatCtx->streams[videoStream]->codec;

	// Find the decoder for the video stream
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec == NULL)
	{
		fprintf(stderr, "Unsupported codec!\n");
		return -1; // Codec not found
	}
	// Open codec
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
	    perror("Error, avcodec_open2\n");
		return -1; // Could not open codec
	}

	// Allocate video frame
	pFrame = avcodec_alloc_frame();

	// Allocate an AVFrame structure
	pFrameRGB = avcodec_alloc_frame();
	if (pFrameRGB == NULL) {
	    perror("Error, avcodec_alloc_frame\n");
		return -1;
	}

	// Determine required buffer size and allocate buffer
	numBytes = avpicture_get_size(PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);
	buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));

	// Assign appropriate parts of buffer to image planes in pFrameRGB
	// Note that pFrameRGB is an AVFrame, but AVFrame is a superset
	// of AVPicture
	avpicture_fill((AVPicture *) pFrameRGB, buffer, PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);
	struct SwsContext *swsContext;
	swsContext = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width,
			pCodecCtx->height, PIX_FMT_BGR24, SWS_BICUBIC, NULL, NULL, NULL);
	// Read frames and save first five frames to disk
	int i = 0;
	while (av_read_frame(pFormatCtx, &packet) >= 0) {
		// Is this a packet from the video stream?
		if (packet.stream_index == videoStream) {
		    if (i++ > 5) {
    			// Decode video frame
    			avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);
    			// Did we get a video frame?
    			if (frameFinished) {
    				// Convert the image from its native format to RGB
    				sws_scale(swsContext, pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);
    				// Save the frame to disk
    				printf("save pic\n");
    				SaveFrame(pFrameRGB, pCodecCtx->width, pCodecCtx->height, i, saveImagePath.c_str());
    				break;
    			}
		    }
		}
		// Free the packet that was allocated by av_read_frame
		av_free_packet(&packet);
	}
	// Free the RGB image
	av_free(buffer);
	av_free(pFrameRGB);
	// Free the YUV frame
	av_free(pFrame);
	// Close the codec
	avcodec_close(pCodecCtx);
	// Close the video file
	av_close_input_file(pFormatCtx);

	return 0;            
}

int VideoFileParser::writeBMPhead(int nWidth,int nHeight,FILE*fp1)
{
     BmpHead m_BMPHeader;        
     char bfType[2]={'B','M'};
     m_BMPHeader.imageSize=3*nWidth*nHeight+54;
     m_BMPHeader.blank=0;
     m_BMPHeader.startPosition=54;
 
     fwrite(bfType,1,sizeof(bfType),fp1);
     fwrite(&m_BMPHeader.imageSize,1,sizeof(m_BMPHeader.imageSize),fp1);
     fwrite(&m_BMPHeader.blank,1,sizeof(m_BMPHeader.blank),fp1);
     fwrite(&m_BMPHeader.startPosition,1,sizeof(m_BMPHeader.startPosition),fp1);
        
     InfoHead  m_BMPInfoHeader;
     m_BMPInfoHeader.Length=40; 
     m_BMPInfoHeader.width=nWidth;
     m_BMPInfoHeader.height=nHeight;
     m_BMPInfoHeader.colorPlane=1;
     m_BMPInfoHeader.bitColor=24;
     m_BMPInfoHeader.zipFormat=0;
     m_BMPInfoHeader.realSize=3*nWidth*nHeight;
     m_BMPInfoHeader.xPels=0;
     m_BMPInfoHeader.yPels=0;
     m_BMPInfoHeader.colorUse=0;
     m_BMPInfoHeader.colorImportant=0;
 
     fwrite(&m_BMPInfoHeader.Length,1,sizeof(m_BMPInfoHeader.Length),fp1);
     fwrite(&m_BMPInfoHeader.width,1,sizeof(m_BMPInfoHeader.width),fp1);
     fwrite(&m_BMPInfoHeader.height,1,sizeof(m_BMPInfoHeader.height),fp1);
     fwrite(&m_BMPInfoHeader.colorPlane,1,sizeof(m_BMPInfoHeader.colorPlane),fp1);
     fwrite(&m_BMPInfoHeader.bitColor,1,sizeof(m_BMPInfoHeader.bitColor),fp1);
     fwrite(&m_BMPInfoHeader.zipFormat,1,sizeof(m_BMPInfoHeader.zipFormat),fp1);
     fwrite(&m_BMPInfoHeader.realSize,1,sizeof(m_BMPInfoHeader.realSize),fp1);
     fwrite(&m_BMPInfoHeader.xPels,1,sizeof(m_BMPInfoHeader.xPels),fp1);
     fwrite(&m_BMPInfoHeader.yPels,1,sizeof(m_BMPInfoHeader.yPels),fp1);
     fwrite(&m_BMPInfoHeader.colorUse,1,sizeof(m_BMPInfoHeader.colorUse),fp1);
     fwrite(&m_BMPInfoHeader.colorImportant,1,sizeof(m_BMPInfoHeader.colorImportant),fp1);
     return 0;
}

int VideoFileParser::SaveFrame(AVFrame *pFrame, int width, int height, int iFrame, const char* filePath)
{
    printf("SaveFrame, width[%d], height[%d], pFrame->linesize[0][%d]\n", width, height, pFrame->linesize[0]);
	FILE *pFile;
	char szFilename[1024];
	int y;

	// Open file
	if (!filePath) {
	    perror("SaveFrame error, path is NULL\n");
	    return -1;    
	}
	    
	sprintf(szFilename, "%s.bmp", filePath);     
	pFile = fopen(szFilename, "wb");
	if (pFile == NULL)
		return -1;

	// Write header
    writeBMPhead(width, height, pFile);
	// Write pixel data
	for (y = height; y > 0; y--)
		fwrite(pFrame->data[0] + y * pFrame->linesize[0], 1, width * 3, pFile);
	// Close file
	fclose(pFile);
	return 0;
}
    
}; //namespace Hippo