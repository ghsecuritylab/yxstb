
#pragma once

#ifndef __DLNAFILELIST_H__
#define __DLNAFILELIST_H__


#include <string>
#include "Message.h"
#include "MessageTypes.h"
#include "MessageHandler.h"
#include <list>
#include "json/json_public.h"

namespace Hippo {

enum DmsScanStatus{
        Scan_NotStart,
        Scan_Running,
        Scan_Finish,
        Scan_stop,
};
class DLNAAudioFileartistList{
public:
    std::string artist;		//O	string(30)	演唱者
    int mun;
    //std::
    DLNAAudioFileartistList();
    ~DLNAAudioFileartistList();

protected:
private:

};
class DLNAAudioFilealbumList{
public:
    std::string album;		//O	string(30)	演唱者
    int mun;
    //std::
    DLNAAudioFilealbumList();
    ~DLNAAudioFilealbumList();

protected:
private:

};

class DLNAFileList{
public:
    std::string filename;
    std::string filepath;
    std::string objectID;
    std::string classID;
    std::string size;
    std::string bitrate;
    int duration;
    std::string protocolInfo;
    std::string resolution;
    std::string colorDepth;
    std::string contentSource;
    std::string date;
    std::string artist;		//O	string(30)	演唱者
    std::string album;		//O	string(30)	专辑
    std::string year;		//	O	string(30)	音乐年份
    std::string genre;		//O	string(30)	音乐流派
    std::string copyright;	//	O	string(30)	版权信息
    std::string composer;	//O	string		作曲
    std::string sampleRate;//:	O	string(30)	音频采样率。
    std::string coverURL;	//	O	string(30)	mp3文件的封面海报地址，

    //std::
    DLNAFileList();
    ~DLNAFileList();

protected:
private:

};

class DLNADirList  {
public:
//    void Startscan();
//    void stopscan();
    //std::string dirname;
    //std::string dmsname;
    std::list<DLNAFileList *>    filelist;

    std::string objectID;
    std::string parentID;
    std::string filename;
    std::string classID;
    int vmun;
    int amun;
    int pmun;
    int Isp;
    int Isa;
    int Isv;
    DLNADirList();
    ~DLNADirList();

protected:
    friend class DLNAFileList;

private:


};

class DLNARootList  {
public:
    std::list<DLNADirList *>    dirlist;
    std::list<DLNAAudioFileartistList *>    artistList;
    std::list<DLNAAudioFilealbumList *>    albumList;

    pthread_t m_scanThreadID;
    DmsScanStatus m_ScanStatus;
    char dmsname[128];
    char DirectoryObjectID[128];//scan
    char keyword[128];
    int vtotal_file;
    int atotal_file;
    int ptotal_file;
    int total_file;
    int total_dir;
    int total_all;
    int stopflag;
    int gettype;//scan
    int directory;//scan
    int lastindex;//scan
    int searchtype;
    int FilterAudiotype;
    char Audiokeyword[128];

    json_object *search_array;

    void Startscan(void);
    //void stopscan(void);
    DLNARootList();
    ~DLNARootList();
    void GetDmsFile(char *deviceID,int type,int index,int count,char *out,int len);
    void GetDmsDirectoryFile(char *deviceID,char *DirObjectID,int type,int index,int count,char *out,int len);
    void GetDmsDirectory(char *deviceID,int type,int index,int count,char *out,int len);
    int DmsGetCount(void);
    int SearchFile(void);
    int IsNeedNile(char *name);
    void GetSearchFile(int index,int count,char *out,int len);
    void GetDmsAudioFile(char *deviceID,int type,int index,int count,char *out,int len);

protected:
    friend class DLNADirList;
    friend class DLNAFileList;
    friend class DLNAAudioFileartistList;
    friend class DLNAAudioFilealbumList;

private:
    void scandir(char *dirstr);
    void adddiritem(DLNADirList *itdir );
    void addfileitem(char *dirstr,DLNAFileList *itfile );
    void printfdiritem(int detail);
    void printffileitem(char *dirstr, int detail);
    void DmsFillResult(json_object *my_array,int totnum, int num, char *value);


};


}// namespace Hippo
#endif //__DLNAFILELIST_H__
