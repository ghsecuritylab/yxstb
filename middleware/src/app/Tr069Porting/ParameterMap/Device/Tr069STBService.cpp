#include "Tr069STBService.h"

#include "Tr069FunctionCall.h"

#include <string.h>

/*------------------------------------------------------------------------------
	֧�ֵ�������Э�飬������Ӧ�ô������б���ѡ��ͬʱ֧��ʱ�Զ��Ÿ�����
	��RTSP����֧�� RTSP
	��IGMP�� ��֧��IGMPv1
	��IGMPv2�� ��֧��IGMPv2 and lower versions
	��IGMPv3�� ��֧�� IGMPv3 and lower versions
 ------------------------------------------------------------------------------*/
static int getPortStreamingControlProtocols(char* value, unsigned int size)
{
    if (size <= strlen("RTSP,IGMP,IGMPv")) 
        return -1;    
    strcpy(value, "RTSP,IGMP,IGMPv2");
    
    return 0;
}

/*------------------------------------------------------------------------------
	֧�ֵĴ����Э�飬������Ӧ�ô������б���ѡ��ͬʱ֧��ʱ�Զ��Ÿ�����
	��UDP��
	��TCP��
	��RTP��
	��HTTP��
 ------------------------------------------------------------------------------*/
static int getPortStreamingTransportProtocols(char *value, unsigned int size)
{
    if (size <= strlen("UDP,TCP,HTTP")) 
        return -1;
    strcpy(value, "UDP,TCP,HTTP");
    
    return 0;
}

/*------------------------------------------------------------------------------
	֧�ֵĴ�������Э�飬������Ӧ�ô������б���ѡ��ͬʱ֧��ʱ�Զ��Ÿ�����
	��RTCP����֧��RTCPЭ��
	��AL-FEC����֧��Ӧ�ò�ǰ�����
 ------------------------------------------------------------------------------*/
static int getPortStreamingTransportControlProtocols(char *value, unsigned int size)
{
    if (size <= strlen("RTCP")) 
        return -1;
    strcpy(value, "RTCP");
        
    return 0;
}

/*------------------------------------------------------------------------------
	֧�ֵ�����Э�飬������Ӧ�ô������б���ѡ��ͬʱ֧��ʱ�Զ��Ÿ�����
	��HTTP��
	��HTTPS��
	��FTP��
	��FTPS�� FTP/SSL [7]
	��SFTP�� SSH file transfer protoco
	��TFTP��
 ------------------------------------------------------------------------------*/
static int getPortDownloadTransportProtocols(char *value, unsigned int size)
{
    if (size <= strlen("HTTP,FTP")) 
        return -1;
    strcpy(value, "HTTP,FTP");

    return 0;
}

/*------------------------------------------------------------------------------
	֧�ֵ�����װЭ�飬������Ӧ�ô������б���ѡ��ͬʱ֧��ʱ�Զ��Ÿ�����
	��None��
	��MPEG1-SYS��
	��MPEG2-PS��
	��VOB��
	��MPEG2-TS��
 ------------------------------------------------------------------------------*/
static int getPortMultiplexTypes(char *value, unsigned int size)
{
    if (size <= strlen("MPEG2-TS")) 
        return -1;
    strcpy(value, "MPEG2-TS");

    return 0;
}

/*------------------------------------------------------------------------------
	Describes the maximum de-jittering buffer size, in bytes, supported by the STB.
	A value of -1 indicates no specific limit on the buffer size.
 ------------------------------------------------------------------------------*/
static int getPortMaxDejitteringBufferSize(char *value, unsigned int size)
{
    if (size <= strlen("5242880")) 
        return -1;
    strcpy(value, "5242880");

    return 0;
}

/*------------------------------------------------------------------------------
	֧�ֵ���Ƶ�����׼��������Ӧ�ô������б���ѡ��ͬʱ֧��ʱ�Զ��Ÿ�����
	��MPEG1-Part3-Layer2�� [21]
	��MPEG1-Part3-Layer3�� [21]
	��MPEG2-Part3-Layer2�� [24]
	��MPEG2-Part3-Layer3�� [24]
	��MP3-Surround��
	��DOLBY-AC3�� Dolby Digital (AC-3) [36]
	��DOLBY-DD+�� Dolby Digital Plus [36]
	��DTS�� Digital Theatre System
	��DTS-HD��
 ------------------------------------------------------------------------------*/
static int getPortAudioStandards(char *value, unsigned int size)
{
    if (size <= strlen("MPEG1-Part3-Layer2,MPEG1-Part3-Layer3,MPEG2-Part3-Layer2,MPEG2-Part3-Layer3")) 
        return -1;
    strcpy(value, "MPEG1-Part3-Layer2,MPEG1-Part3-Layer3,MPEG2-Part3-Layer2,MPEG2-Part3-Layer3");

    return 0;
}

/*------------------------------------------------------------------------------
	֧�ֵ���Ƶ�����׼��������Ӧ�ô������б���ѡ��ͬʱ֧��ʱ�Զ��Ÿ�����
	��MPEG2-Part2�� [22]
	��MPEG4-Part2�� [25]
	��MPEG4-Part10�� Same as MPEG4 AVC
	and H.264 [29]
	��SMPTE-VC-1�� [37]
 ------------------------------------------------------------------------------*/
static int getPortVideoStandards(char *value, unsigned int size)
{
    if (size <= strlen("MPEG2-Part2,MPEG4-Part2,MPEG4-Part10,SMPTE-VC-1")) 
        return -1;
    strcpy(value, "MPEG2-Part2,MPEG4-Part2,MPEG4-Part10,SMPTE-VC-1");

    return 0;
}


/*************************************************
Description: ��ʼ��tr069V1����Ľӿ�,������root.Device.STBService��
Input: ��
Return: ��
 *************************************************/

Tr069STBService::Tr069STBService()
	: Tr069GroupCall("STBService")
{
	/* ���¶����ע�ᵽ��root.Device.STBService  */
    Tr069Call* fun1  = new Tr069FunctionCall("StreamingControlProtocols", getPortStreamingControlProtocols, NULL);
    regist(fun1->name(), fun1);    
    
    Tr069Call* fun2  = new Tr069FunctionCall("StreamingTransportProtocols", getPortStreamingTransportProtocols, NULL);
    regist(fun2->name(), fun2);    
    
    Tr069Call* fun3  = new Tr069FunctionCall("StreamingTransportControlProtocols", getPortStreamingTransportControlProtocols, NULL);
    regist(fun3->name(), fun3);    
    
    Tr069Call* fun4  = new Tr069FunctionCall("DownloadTransportProtocols", getPortDownloadTransportProtocols, NULL);
    regist(fun4->name(), fun4);    
    
    Tr069Call* fun5  = new Tr069FunctionCall("MultiplexTypes", getPortMultiplexTypes, NULL);
    regist(fun5->name(), fun5);    
    
    Tr069Call* fun6  = new Tr069FunctionCall("MaxDejitteringBufferSize", getPortMaxDejitteringBufferSize, NULL);
    regist(fun6->name(), fun6);    
    
    Tr069Call* fun7  = new Tr069FunctionCall("AudioStandards", getPortAudioStandards, NULL);
    regist(fun7->name(), fun7);    
    
    Tr069Call* fun8  = new Tr069FunctionCall("VideoStandards", getPortVideoStandards, NULL);
    regist(fun8->name(), fun8);


}

Tr069STBService::~Tr069STBService()
{
}
