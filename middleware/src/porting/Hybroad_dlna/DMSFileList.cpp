#include <string>
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "AppSetting.h"
#include "DMSFileList.h"
#include "Hippo_api.h"
#include "DlnaAssertions.h"
#include "CodeTransform.h"

#include "app_heartbit.h"
#include "Assertions.h"
#include "../../app/Assertions.h"
#include "mid/mid_tools.h"
#include "mid_sys.h"
#include "ipanel_event.h"
#include "json/json_public.h"

extern "C" {

int Raw_Dmc_HuaweiJse_V1R5(const char *func, const char *para, char *value, int len);
}

namespace Hippo {
void *runStartscan(void *p)
{
    DLNARootList *it = (DLNARootList *)p;
    it->Startscan();

    return NULL;
}

static std::list<DLNARootList *> DLNArootlist;
extern "C" {

int StartscanDmsList( const char *aFieldName,const char *deviceID,char *out,int len)
{
	DLNA_LOG(" StartscanDmsList in aFieldName %s deviceID=%s\n",aFieldName,deviceID);
//sleep(10);
    char deviceuid[128] = {0};
    if(deviceID)
    {
            memcpy(deviceuid,deviceID,strlen(deviceID));

    }
#if 1
    if(!deviceID)
    {
        char value[1024*6] = {0};
        struct json_object *para_obj = NULL,*array_obj = NULL,*idx_obj = NULL,*idx_obj_item = NULL;
        int i = 0,index = 0,count = 1;
        while(1){
        Raw_Dmc_HuaweiJse_V1R5("dlna.getDmsCount",NULL ,value , sizeof(value));
        //printf("value %s \n",value);
        if(atoi(value))
        {//sleep(10);
                        break;
        }
        /*para_obj = json_tokener_parse(value);
        array_obj = json_object_object_get(para_obj, "count");

        if(json_object_get_int(array_obj)){
            json_object_put(para_obj);
            }
            json_object_put(para_obj);*/
            sleep(1);
        }
        para_obj = json_object_new_object();

        memset(value,0,sizeof(value));
        json_object_object_add(para_obj, "index", json_object_new_int(0));
        json_object_object_add(para_obj, "count", json_object_new_int(1));
        //printf("%s \n",json_object_get_string(para_obj));
        Raw_Dmc_HuaweiJse_V1R5("dlna.getDmsList",json_object_get_string(para_obj) ,value , sizeof(value));
        //printf("value %s \n",value);
        json_object_put(para_obj);

        para_obj = json_tokener_parse(value);
        //struct array_list* json_array = json_object_get_array(para_obj) ;
        array_obj = json_object_object_get(para_obj, "dmsList");
        int json_array_len = json_object_array_length(array_obj);
        for( i = 0;i < json_array_len ;i++)
        {
            idx_obj = json_object_array_get_idx(array_obj, i);
            //printf("%s\n",json_object_get_string(idx_obj));
            idx_obj_item = json_object_object_get(idx_obj, "friendlyName");
                        //printf("friendlyName%s\n",json_object_get_string(idx_obj_item));
            idx_obj_item = json_object_object_get(idx_obj, "deviceID");
                        //printf("deviceID%s\n",json_object_get_string(idx_obj_item));

            memcpy(deviceuid,json_object_get_string(idx_obj_item),strlen(json_object_get_string(idx_obj_item)));

        }
        //getchar();
        //return -1;
    }
#endif
    //Raw_Dmc_HuaweiJse_V1R5(const char *func, const char *para, char *value, int len)
    std::list<DLNARootList*>::iterator itroot;
    itroot= DLNArootlist.begin();
//            printf("%s %d \n",__FILE__,__LINE__);

    while(itroot != DLNArootlist.end())
    {
        if(!strcmp((*itroot)->dmsname,deviceuid))
        {
            (*itroot)->m_ScanStatus = Scan_Finish;
            return 0;
        }
        itroot++;
    }
//            printf("%s %d \n",__FILE__,__LINE__);

    //it->Startscan(deviceID);
    DLNARootList *it = new DLNARootList;
    memcpy(it->dmsname,deviceuid,strlen(deviceuid));
    DLNArootlist.push_back(it);
//        printf("%s %d \n",__FILE__,__LINE__);

    if (pthread_create(&(it->m_scanThreadID),NULL,runStartscan,(void *)it) ){
        LogUserOperError("pthread create runScandir error\n");
        it->m_ScanStatus = Scan_NotStart;
        return -1;
    }
    pthread_detach(it->m_scanThreadID);
    return 0;
}
int StopscanDmsList( const char* aFieldName,const char *deviceID,char *out,int len)
{
    std::list<DLNARootList*>::iterator itroot;
    itroot= DLNArootlist.begin();
    while(itroot != DLNArootlist.end())
    {
        if(!strcmp((*itroot)->dmsname,deviceID))
        {
            (*itroot)->stopflag = 1;
            return 0;
        }
        itroot++;
    }
    return -1;
}
int GetscanDmsListStatus( const char* aFieldName,const char *deviceID,char *out,int len)
{
    if(len<10)
        return -1;
    std::list<DLNARootList*>::iterator itroot;
    itroot= DLNArootlist.begin();
    while(itroot != DLNArootlist.end())
    {
        if(!strcmp((*itroot)->dmsname,deviceID))
        {
            sprintf(out,"{\"Status\":\"%d\"}", (*itroot)->m_ScanStatus);
            return 0;
        }
        itroot++;
    }
    sprintf(out,"{\"Status\":\"%d\"}",Scan_NotStart);
    return -1;
}

int DeletescanDmsList( const char* aFieldName,const char *deviceID,char *out,int len)
{
    if (!deviceID)
        return -1;
    std::list<DLNARootList*>::iterator itroot;
    itroot= DLNArootlist.begin();
    while(itroot != DLNArootlist.end())
    {
        //printf("(*itroot)->dmsname = %s   deviceID = %s\n",(*itroot)->dmsname,deviceID);
        if(!strcmp((*itroot)->dmsname,deviceID))
        {
            (*itroot)->stopflag = 1;
            while((*itroot)->m_ScanStatus != Scan_Finish)
            {
                sleep(1);
            }
            delete *itroot;
            DLNArootlist.erase(itroot);
            return 0;
        }
        itroot++;
    }
    return -1;
}
/*{"deviceID":"xxxxxx","Gettype":"Video/Audio/Image","Directory":"YES/NO","DirectoryObjectID":"xxxx","matchRule":"Exact/Acronym","keyWord":"keyWord"}*/
/*{"deviceID":"xxxxxx","Gettype":"Video/Audio/Image","Directory":"NO","DirectoryObjectID":"NULL","matchRule":"NULL","keyWord":"NULL"}获取Gettype类型文件*/
/*{"deviceID":"xxxxxx","Gettype":"Video/Audio/Image","Directory":"YES","DirectoryObjectID":"NULL","matchRule":"NULL","keyWord":"NULL"}获取Gettype类型的最后一级目录*/
/*{"deviceID":"xxxxxx","Gettype":"Video/Audio/Image","Directory":"NO","DirectoryObjectID":"xxxx","matchRule":"NULL","keyWord":"NULL"}获取xxxx文件夹下的Gettype类型的文件*/
/*{"deviceID":"xxxxxx","Gettype":"Video/Audio/Image","Directory":"NO","DirectoryObjectID":"NULL","matchRule":"Exact/Acronym","keyWord":"keyWord"}获取xxxx文件夹下的Gettype类型的文件*/
int SetDmsfileFilter(const char* aFieldName,const char *in,char *out,int len)
{
//printf("1111\n");
    DLNA_LOG("%s %d  in=%s \n",__FILE__,__LINE__,in);

    if (!in)
        return -1;
    char *deviceID = NULL;
    //printf("%s %d  \n",__FILE__,__LINE__);

    struct json_object *parse_obj = NULL,*in_obj = NULL;
    parse_obj = json_tokener_parse(in);
    if (!parse_obj){
        json_object_put(parse_obj);
        DLNA_LOG("%s %d \n",__FILE__,__LINE__);
        return -1;
    }
    in_obj = json_object_object_get(parse_obj, "deviceID");
    if (!in_obj){
        json_object_put(parse_obj);
        DLNA_LOG("%s %d \n",__FILE__,__LINE__);
        return -1;
    }
    deviceID = (char *)json_object_get_string(in_obj);
    std::list<DLNARootList*>::iterator itroot;
    itroot= DLNArootlist.begin();
    while(itroot != DLNArootlist.end())
    {//printf("%s %s \n",(*itroot)->dmsname,deviceID);
        if(!strcmp((*itroot)->dmsname,deviceID))
        {
            break;
        }
        itroot++;
    }    //printf("%s %d  \n",__FILE__,__LINE__);

    if (itroot == DLNArootlist.end()){
        json_object_put(parse_obj);
        DLNA_LOG("%s %d \n",__FILE__,__LINE__);
        return -1;
    }
    //printf("%s %d  \n",__FILE__,__LINE__);

    in_obj = json_object_object_get(parse_obj, "Gettype");
    if (!in_obj){
        json_object_put(parse_obj);
        DLNA_LOG("%s %d \n",__FILE__,__LINE__);
        return -1;
    }    //printf("%s %d  \n",__FILE__,__LINE__);

    deviceID = (char *)json_object_get_string(in_obj);
    if (0 == strcmp(deviceID,"Video")) {
        (*itroot)->gettype = 3;
    }
    else if (0 == strcmp(deviceID,"Audio")) {
        (*itroot)->gettype = 2;
    }
    else if (0 == strcmp(deviceID,"Image")) {
        (*itroot)->gettype = 1;
    }
    else {
    }    //printf("%s %d  \n",__FILE__,__LINE__);

    in_obj = json_object_object_get(parse_obj, "Directory");
    if (!in_obj){
        json_object_put(parse_obj);
        DLNA_LOG("%s %d \n",__FILE__,__LINE__);
        return -1;
    }   // printf("%s %d  \n",__FILE__,__LINE__);

    deviceID = (char *)json_object_get_string(in_obj);
    if (0 == strcmp(deviceID,"YES")) {
        (*itroot)->directory = 1;
    }
    else{
        (*itroot)->directory = 0;
    }    //printf("%s %d  \n",__FILE__,__LINE__);

    in_obj = json_object_object_get(parse_obj, "DirectoryObjectID");
    if (!in_obj){
        json_object_put(parse_obj);
        DLNA_LOG("%s %d \n",__FILE__,__LINE__);
        return -1;
    }    //printf("%s %d  \n",__FILE__,__LINE__);

    deviceID = (char *)json_object_get_string(in_obj);
    strcpy((*itroot)->DirectoryObjectID ,deviceID);
    //printf("%s %d  \n",__FILE__,__LINE__);

    in_obj = json_object_object_get(parse_obj, "matchRule");//搜索类型
    if (!in_obj){
        json_object_put(parse_obj);
        DLNA_LOG("%s %d \n",__FILE__,__LINE__);
        return -1;
    }    //printf("%s %d  \n",__FILE__,__LINE__);

    deviceID = (char *)json_object_get_string(in_obj);
    if (0 == strcmp(deviceID,"Exact")) {
        (*itroot)->searchtype = 1;//确切
    }
    else if (0 == strcmp(deviceID,"Acronym")) {
        (*itroot)->searchtype = 2;//首字母
    }
    else {
        (*itroot)->searchtype = 0;
    }    //printf("%s %d  \n",__FILE__,__LINE__);

    in_obj = json_object_object_get(parse_obj, "keyWord");//搜索关键字
    if (!in_obj){
        json_object_put(parse_obj);
        DLNA_LOG("%s %d \n",__FILE__,__LINE__);
        return -1;
    }    //printf("%s %d  \n",__FILE__,__LINE__);
    deviceID = (char *)json_object_get_string(in_obj);
    strcpy((*itroot)->keyword ,deviceID);

    (*itroot)->lastindex = 0;
    json_object_put(parse_obj);
    sprintf(out,"{\"Count\":\"%d\"}",(*itroot)->DmsGetCount());
    DLNA_LOG("%s %d  out = %s\n",__FILE__,__LINE__,out);

    return 0;
}
/*{"deviceID":"xxxxxx"}
获取deviceID为"xxxxxx"所有artist的类型,和每种的数量*/
int GetDmsAllAudioartist(const char* aFieldName,const char *in,char *out,int len)
{
DLNA_LOG("%s %d  in=%s \n",__FILE__,__LINE__,in);

    if (!in)
        return -1;
    char *deviceID = NULL;
    //printf("%s %d  \n",__FILE__,__LINE__);

    struct json_object *parse_obj = NULL,*in_obj = NULL;
    parse_obj = json_tokener_parse(in);
    if (!parse_obj){
        json_object_put(parse_obj);
        DLNA_LOG("%s %d \n",__FILE__,__LINE__);
        return -1;
    }
    in_obj = json_object_object_get(parse_obj, "deviceID");
    if (!in_obj){
        json_object_put(parse_obj);
        DLNA_LOG("%s %d \n",__FILE__,__LINE__);
        return -1;
    }
    deviceID = (char *)json_object_get_string(in_obj);
    std::list<DLNARootList*>::iterator itroot;
    itroot= DLNArootlist.begin();
    while(itroot != DLNArootlist.end())
    {//printf("%s %s \n",(*itroot)->dmsname,deviceID);
        if(!strcmp((*itroot)->dmsname,deviceID))
        {
            break;
        }
        itroot++;
    }    //printf("%s %d  \n",__FILE__,__LINE__);

    if (itroot == DLNArootlist.end()){
        json_object_put(parse_obj);
        DLNA_LOG("%s %d \n",__FILE__,__LINE__);
        return -1;
    }
    json_object_put(parse_obj);

    std::list<DLNAAudioFileartistList *>::iterator    artistit;
    artistit = (*itroot)->artistList.begin();
    sprintf(out,"{allmun:%d}",(*itroot)->artistList.size());
    while(artistit != (*itroot)->artistList.end() && strlen(out)<(unsigned)len) {
        sprintf(out+strlen(out),",{%s:%d}",((*artistit)->artist.c_str()),(*artistit)->mun);
        artistit++;
    }
    DLNA_LOG("%s %d  out = %s\n",__FILE__,__LINE__,out);

    return 0;

}
/*{"deviceID":"xxxxxx"}
获取deviceID为"xxxxxx"所有albumt的类型,和每种的数量*/

int GetDmsAllAudioalbumt(const char* aFieldName,const char *in,char *out,int len)
{
DLNA_LOG("%s %d  in=%s \n",__FILE__,__LINE__,in);

    if (!in)
        return -1;
    char *deviceID = NULL;
    //printf("%s %d  \n",__FILE__,__LINE__);

    struct json_object *parse_obj = NULL,*in_obj = NULL;
    parse_obj = json_tokener_parse(in);
    if (!parse_obj){
        json_object_put(parse_obj);
        DLNA_LOG("%s %d \n",__FILE__,__LINE__);
        return -1;
    }
    in_obj = json_object_object_get(parse_obj, "deviceID");
    if (!in_obj){
        json_object_put(parse_obj);
        DLNA_LOG("%s %d \n",__FILE__,__LINE__);
        return -1;
    }
    deviceID = (char *)json_object_get_string(in_obj);
    std::list<DLNARootList*>::iterator itroot;
    itroot= DLNArootlist.begin();
    while(itroot != DLNArootlist.end())
    {//printf("%s %s \n",(*itroot)->dmsname,deviceID);
        if(!strcmp((*itroot)->dmsname,deviceID))
        {
            break;
        }
        itroot++;
    }    //printf("%s %d  \n",__FILE__,__LINE__);

    if (itroot == DLNArootlist.end()){
        json_object_put(parse_obj);
        DLNA_LOG("%s %d \n",__FILE__,__LINE__);
        return -1;
    }
    json_object_put(parse_obj);

    std::list<DLNAAudioFilealbumList *>::iterator    albumit;
    albumit = (*itroot)->albumList.begin();
    sprintf(out,"{allmun:%d}",(*itroot)->albumList.size());
    while(albumit != (*itroot)->albumList.end() && strlen(out)<(unsigned)len) {
        sprintf(out+strlen(out),",{%s:%d}",((*albumit)->album.c_str()),(*albumit)->mun);
        albumit++;
    }
    DLNA_LOG("%s %d  out = %s\n",__FILE__,__LINE__,out);

    return 0;

}

/*{"deviceID":"xxxxxx","Filtertype":"artist/album","keyWord":"keyWord"}
获取artist/album 是keyWord的文件返回符合的文件数量*/
int SetDmsAudiofileFilter(const char* aFieldName,const char *in,char *out,int len)
{
    DLNA_LOG("%s %d  in=%s \n",__FILE__,__LINE__,in);

    if (!in)
        return -1;
    char *deviceID = NULL;
    //printf("%s %d  \n",__FILE__,__LINE__);

    struct json_object *parse_obj = NULL,*in_obj = NULL;
    parse_obj = json_tokener_parse(in);
    if (!parse_obj){
        json_object_put(parse_obj);
        DLNA_LOG("%s %d \n",__FILE__,__LINE__);
        return -1;
    }
    in_obj = json_object_object_get(parse_obj, "deviceID");
    if (!in_obj){
        json_object_put(parse_obj);
        DLNA_LOG("%s %d \n",__FILE__,__LINE__);
        return -1;
    }
    deviceID = (char *)json_object_get_string(in_obj);
    std::list<DLNARootList*>::iterator itroot;
    itroot= DLNArootlist.begin();
    while(itroot != DLNArootlist.end())
    {//printf("%s %s \n",(*itroot)->dmsname,deviceID);
        if(!strcmp((*itroot)->dmsname,deviceID))
        {
            break;
        }
        itroot++;
    }    //printf("%s %d  \n",__FILE__,__LINE__);

    if (itroot == DLNArootlist.end()){
        json_object_put(parse_obj);
        DLNA_LOG("%s %d \n",__FILE__,__LINE__);
        return -1;
    }
    //printf("%s %d  \n",__FILE__,__LINE__);

    in_obj = json_object_object_get(parse_obj, "Filtertype");
    if (!in_obj){
        json_object_put(parse_obj);
        DLNA_LOG("%s %d \n",__FILE__,__LINE__);
        return -1;
    }    //printf("%s %d  \n",__FILE__,__LINE__);
    deviceID = (char *)json_object_get_string(in_obj);
    if (0 == strcmp(deviceID,"artist")) {
        (*itroot)->FilterAudiotype = 1;
    }
    else {
        (*itroot)->FilterAudiotype = 2;
    }

    in_obj = json_object_object_get(parse_obj, "keyWord");
    if (!in_obj){
        json_object_put(parse_obj);
        DLNA_LOG("%s %d \n",__FILE__,__LINE__);
        return -1;
    }    //printf("%s %d  \n",__FILE__,__LINE__);
    deviceID = (char *)json_object_get_string(in_obj);
    strcpy((*itroot)->Audiokeyword ,deviceID);


    (*itroot)->lastindex = 0;

    sprintf(out,"{\"Count\":\"%d\"}",0);

    if((*itroot)->FilterAudiotype == 1) {
        std::list<DLNAAudioFileartistList*>::iterator it;
        it = (*itroot)->artistList.begin();
        while(it != (*itroot)->artistList.end()) {
            if(strcmp(deviceID,(*it)->artist.c_str()) == 0){
                sprintf(out,"{\"Count\":\"%d\"}",(*it)->mun);
                break;
            }
            it++;
        }
    }else if((*itroot)->FilterAudiotype == 2) {
        std::list<DLNAAudioFilealbumList*>::iterator it;
        it = (*itroot)->albumList.begin();
        while(it != (*itroot)->albumList.end()) {
            if(strcmp(deviceID,(*it)->album.c_str()) == 0){
                sprintf(out,"{\"Count\":\"%d\"}",(*it)->mun);
                break;

            }
            it++;
        }
    }

    DLNA_LOG("%s %d  out = %s\n",__FILE__,__LINE__,out);
    json_object_put(parse_obj);

    return 0;

}
/*{"deviceID","xxxxxx","index":0,"count":20}*/
int GetDmsFileList(const char* aFieldName,const char *in,char *out,int len)
{
    if (!in)
        return -1;
    DLNA_LOG("%s %d  in=%s \n",__FILE__,__LINE__,in);
    char *deviceID = NULL;
    int index= 0,count = 0;
    struct json_object *parse_obj = NULL,*in_obj = NULL;
    parse_obj = json_tokener_parse(in);
    if (!parse_obj){
        json_object_put(parse_obj);
        return -1;
    }
    in_obj = json_object_object_get(parse_obj, "deviceID");
    if (!in_obj){
        json_object_put(parse_obj);
        return -1;
    }
    deviceID = (char *)json_object_get_string(in_obj);
    std::list<DLNARootList*>::iterator itroot;
    itroot= DLNArootlist.begin();
    while(itroot != DLNArootlist.end())//找到dms
    {
        if(!strcmp((*itroot)->dmsname,deviceID))
        {
            break;
        }
        itroot++;
    }
    //printf("%s %d  \n",__FILE__,__LINE__);
    if (itroot == DLNArootlist.end()){
        json_object_put(parse_obj);
        return -1;
    }
    //printf("%s %d  \n",__FILE__,__LINE__);
    in_obj = json_object_object_get(parse_obj, "index");
    if (!in_obj){
        json_object_put(parse_obj);
        return -1;
    }//printf("%s %d  \n",__FILE__,__LINE__);
    index = json_object_get_int(in_obj);

    in_obj = json_object_object_get(parse_obj, "count");
    if (!in_obj){
        json_object_put(parse_obj);
        return -1;
    }//printf("%s %d  \n",__FILE__,__LINE__);
    count = json_object_get_int(in_obj);
    //printf("%s %d  \n",__FILE__,__LINE__);
    if((*itroot)->directory){//取目录
        (*itroot)->GetDmsDirectory(deviceID,(*itroot)->gettype,index,count,out,len);
    }
    else if((*itroot)->searchtype && (0 != strcmp((*itroot)->keyword,"NULL"))) {
        (*itroot)->GetSearchFile(index,count,out,len);
    }
    else if(0 == strcmp((*itroot)->DirectoryObjectID,"NULL")){//取文件
        (*itroot)->GetDmsFile(deviceID,(*itroot)->gettype,index,count,out,len);
    }
    else {//取目录下文件
        (*itroot)->GetDmsDirectoryFile(deviceID,(*itroot)->DirectoryObjectID,(*itroot)->gettype,index,count,out,len);
    }
    DLNA_LOG("%s %d  out = %s\n",__FILE__,__LINE__,out);
    json_object_put(parse_obj);
    return 0;
}
/*{"deviceID","xxxxxx","index":0,"count":20}*/
int GetDmsAudioFileList(const char* aFieldName,const char *in,char *out,int len)
{
    if (!in)
        return -1;
    DLNA_LOG("%s %d  in=%s \n",__FILE__,__LINE__,in);
    char *deviceID = NULL;
    int index= 0,count = 0;
    struct json_object *parse_obj = NULL,*in_obj = NULL;
    parse_obj = json_tokener_parse(in);
    if (!parse_obj){
        json_object_put(parse_obj);
        return -1;
    }
    in_obj = json_object_object_get(parse_obj, "deviceID");
    if (!in_obj){
        json_object_put(parse_obj);
        return -1;
    }
    deviceID = (char *)json_object_get_string(in_obj);
    std::list<DLNARootList*>::iterator itroot;
    itroot= DLNArootlist.begin();
    while(itroot != DLNArootlist.end())//找到dms
    {
        if(!strcmp((*itroot)->dmsname,deviceID))
        {
            break;
        }
        itroot++;
    }
    //printf("%s %d  \n",__FILE__,__LINE__);
    if (itroot == DLNArootlist.end()){
        json_object_put(parse_obj);
        return -1;
    }
    //printf("%s %d  \n",__FILE__,__LINE__);
    in_obj = json_object_object_get(parse_obj, "index");
    if (!in_obj){
        json_object_put(parse_obj);
        return -1;
    }//printf("%s %d  \n",__FILE__,__LINE__);
    index = json_object_get_int(in_obj);

    in_obj = json_object_object_get(parse_obj, "count");
    if (!in_obj){
        json_object_put(parse_obj);
        return -1;
    }//printf("%s %d  \n",__FILE__,__LINE__);
    count = json_object_get_int(in_obj);
    //printf("%s %d  \n",__FILE__,__LINE__);
    if( ((*itroot)->FilterAudiotype == 1 || (*itroot)->FilterAudiotype == 2) && (*itroot)->Audiokeyword) {
        (*itroot)->GetDmsAudioFile(deviceID,(*itroot)->FilterAudiotype,index,count,out,len);

    }else {
        json_object_put(parse_obj);
        return -1;
    }

    json_object_put(parse_obj);
    DLNA_LOG("%s %d  out = %s\n",__FILE__,__LINE__,out);
    return 0;
}
void DmsList_jseAPI_Register(ioctl_context_type_e type)
{
        a_Hippo_API_JseRegister( "dlna.StartscanDmsList",           StartscanDmsList, NULL, type );
        a_Hippo_API_JseRegister( "dlna.StopscanDmsList",            StopscanDmsList, NULL, type );
        a_Hippo_API_JseRegister( "dlna.GetscanDmsListStatus",   GetscanDmsListStatus, NULL, type );
        a_Hippo_API_JseRegister( "dlna.DeletescanDmsList", 		DeletescanDmsList, NULL, type );
        a_Hippo_API_JseRegister( "dlna.SetDmsfileFilter", 		SetDmsfileFilter, NULL, type );
        a_Hippo_API_JseRegister( "dlna.GetDmsFileList", 		GetDmsFileList, NULL, type );

        a_Hippo_API_JseRegister( "dlna.GetDmsAllAudioartist",            GetDmsAllAudioartist, NULL, type );
        a_Hippo_API_JseRegister( "dlna.GetDmsAllAudioalbumt",   GetDmsAllAudioalbumt, NULL, type );
        a_Hippo_API_JseRegister( "dlna.SetDmsAudiofileFilter", 		SetDmsAudiofileFilter, NULL, type );
        a_Hippo_API_JseRegister( "dlna.GetDmsAudioFileList", 		GetDmsAudioFileList, NULL, type );

}

}

void DLNARootList::Startscan(void)
{
    DLNA_LOG("scandir start\n");
    m_ScanStatus = Scan_Running;
    scandir(NULL);
    m_ScanStatus = Scan_Finish;
    DLNA_LOG("scandir end\n");

    return ;
}
void DLNARootList::scandir(char *dirstr)
{
    struct json_object *para_obj = NULL,*array_obj = NULL,*idx_obj = NULL,*idx_obj_item = NULL;;
    char value[1024*6] = {0},fileList[1024] = {0},parentID[128] = {0};
    int i = 0,index = 0,count = 10,fileList_len = 0,List_len= 0;
    para_obj = json_object_new_object();
    //printf("this->dmsname = %s\n",this->dmsname);
    if (this->dmsname) {
        json_object_object_add(para_obj, "deviceID", json_object_new_string(this->dmsname));
    }
    else {
            DLNA_LOG("this->dmsname = NULL\n");

    }
    if(dirstr)
    {
        json_object_object_add(para_obj, "containerID", json_object_new_string(dirstr));

    }
    else
    {
        json_object_object_add(para_obj, "containerID", json_object_new_string("-1"));
    }
    json_object_object_add(para_obj, "sort", json_object_new_int(0));
    json_object_object_add(para_obj, "order", json_object_new_int(0));
    //printf(" value =%s\n",json_object_get_string(para_obj));

    Raw_Dmc_HuaweiJse_V1R5("dlna.openFileList",json_object_get_string(para_obj) ,fileList , sizeof(fileList));
    json_object_put(para_obj);
    //printf("openFileList value =%s\n",fileList);


    memset(value,0,sizeof(value));
    para_obj = json_object_new_object();
    json_object_object_add(para_obj, "listID", json_object_new_string(fileList));
    Raw_Dmc_HuaweiJse_V1R5("dlna.getCount",json_object_get_string(para_obj) ,value , sizeof(value));
    json_object_put(para_obj);

    fileList_len = atoi(value);
    for(index = 0;index < (fileList_len + 10); )
    {
        if (stopflag)
            return ;
        memset(value,0,sizeof(value));
        para_obj = json_object_new_object();
        json_object_object_add(para_obj, "listID", json_object_new_string(fileList));
        json_object_object_add(para_obj, "index", json_object_new_int(index));
        json_object_object_add(para_obj, "count", json_object_new_int(count));
        index += count;
        Raw_Dmc_HuaweiJse_V1R5("dlna.getList",json_object_get_string(para_obj) ,value , sizeof(value));
        //获取文件列表

        json_object_put(para_obj);
        //printf("getList value =%s\n",value);

        para_obj = json_tokener_parse(value);
        //struct array_list* json_array = json_object_get_array(para_obj) ;
        array_obj = json_object_object_get(para_obj, "count");
        if(!array_obj)
        {
                continue;
        }
        List_len = json_object_get_int(array_obj);
        //printf("count = %d",List_len);
        if(!List_len)
        {
            break ;
        }
        //解析得到的数量

        array_obj = json_object_object_get(para_obj, "fileList");
        if(!array_obj)
        {
                continue;
        }
        int json_array_len = json_object_array_length(array_obj);
        //得到返回值中数组长度

        for( i = 0;i < json_array_len ;i++)
        {
            if (stopflag)
                return ;

            idx_obj = json_object_array_get_idx(array_obj, i);
            //printf("%s\n",json_object_get_string(idx_obj));

            idx_obj_item = json_object_object_get(idx_obj, "classID");
             if(!idx_obj_item)
             {
                continue;
             }
            //printf("classID %s\n",json_object_get_string(idx_obj_item));
            if(!strcmp( json_object_get_string(idx_obj_item) ,"a2"))//判断是否是目录
            {
                DLNADirList *itdir = new DLNADirList;
                //strncpy(itdir->classID,(char *)json_object_get_string(idx_obj_item),strlen(json_object_get_string(idx_obj_item))<128?strlen(json_object_get_string(idx_obj_item)):127);
                itdir->classID = json_object_get_string(idx_obj_item);
                idx_obj_item = json_object_object_get(idx_obj, "objectID");
                if(idx_obj_item)
                {
                    //printf("objectID %s\n",json_object_get_string(idx_obj_item));
                    //strncpy(itdir->objectID,(char *)json_object_get_string(idx_obj_item),strlen(json_object_get_string(idx_obj_item))<128?strlen(json_object_get_string(idx_obj_item)):127);
                    itdir->objectID = json_object_get_string(idx_obj_item);
                }
                idx_obj_item = json_object_object_get(idx_obj, "parentID");
                if(idx_obj_item)
                {
                    //printf("parentID %s\n",json_object_get_string(idx_obj_item));
                    //strncpy(itdir->parentID,(char *)json_object_get_string(idx_obj_item),strlen(json_object_get_string(idx_obj_item))<128?strlen(json_object_get_string(idx_obj_item)):127);
                    itdir->parentID = json_object_get_string(idx_obj_item);
                    strncpy( parentID,(char *)json_object_get_string(idx_obj_item),strlen(json_object_get_string(idx_obj_item))<128?strlen(json_object_get_string(idx_obj_item)):127);
                }
                idx_obj_item = json_object_object_get(idx_obj, "filename");
                if(idx_obj_item)
                {
                    //printf("filename %s\n",json_object_get_string(idx_obj_item));
                    //strncpy(itdir->filename,(char *)json_object_get_string(idx_obj_item),strlen(json_object_get_string(idx_obj_item))<128?strlen(json_object_get_string(idx_obj_item)):127);
                    itdir->filename = json_object_get_string(idx_obj_item);
                }
                adddiritem(itdir);

                //scandir(itdir->objectID);
                //printf("itdir->objectID :: %s end!!\n",itdir->objectID);
            }
            else{
                //printf("\n\nit is file\n\n");
                DLNAFileList *itfile = new DLNAFileList;
                itfile->classID = json_object_get_string(idx_obj_item);
                 idx_obj_item = json_object_object_get(idx_obj, "objectID");
                if(idx_obj_item)
                {
                    //printf("objectID %s\n",json_object_get_string(idx_obj_item));
                    itfile->objectID = json_object_get_string(idx_obj_item);
                }
                idx_obj_item = json_object_object_get(idx_obj, "filename");
                if(idx_obj_item)
                {
                    //printf("filename %s\n",json_object_get_string(idx_obj_item));
                    itfile->filename = json_object_get_string(idx_obj_item);
                }
                idx_obj_item = json_object_object_get(idx_obj, "size");
                if(idx_obj_item)
                {
                    //printf("size %s\n",json_object_get_string(idx_obj_item));
                    itfile->size = json_object_get_string(idx_obj_item);
                }
                idx_obj_item = json_object_object_get(idx_obj, "bitrate");
                if(idx_obj_item)
                {
                    //printf("bitrate %s\n",json_object_get_string(idx_obj_item));
                    itfile->bitrate = json_object_get_string(idx_obj_item);
                }
                idx_obj_item = json_object_object_get(idx_obj, "duration");
                if(idx_obj_item)
                {
                    //printf("duration %d\n",json_object_get_int(idx_obj_item));
                    itfile->duration = json_object_get_int(idx_obj_item);
                }
                idx_obj_item = json_object_object_get(idx_obj, "protocolInfo");
                if(idx_obj_item)
                {
                    //printf("protocolInfo %s\n",json_object_get_string(idx_obj_item));
                    itfile->protocolInfo = json_object_get_string(idx_obj_item);
                }
                idx_obj_item = json_object_object_get(idx_obj, "filepath");
                if(idx_obj_item)
                {
                    //printf("filepath %s\n",json_object_get_string(idx_obj_item));
                    itfile->filepath = json_object_get_string(idx_obj_item);
                }

				idx_obj_item = json_object_object_get(idx_obj, "resolution");
                if(idx_obj_item)
                {
                    //printf("resolution %s\n",json_object_get_string(idx_obj_item));
                    itfile->resolution = json_object_get_string(idx_obj_item);
                }

				idx_obj_item = json_object_object_get(idx_obj, "colorDepth");
                if(idx_obj_item)
                {
                    //printf("colorDepth %s\n",json_object_get_string(idx_obj_item));
                    itfile->colorDepth = json_object_get_string(idx_obj_item);
                }

				idx_obj_item = json_object_object_get(idx_obj, "contentSource");
                if(idx_obj_item)
                {
                    //printf("contentSource %s\n",json_object_get_string(idx_obj_item));
                    itfile->contentSource = json_object_get_string(idx_obj_item);
                }

				idx_obj_item = json_object_object_get(idx_obj, "date");
                if(idx_obj_item)
                {
                    //printf("date %s\n",json_object_get_string(idx_obj_item));
                    itfile->date = json_object_get_string(idx_obj_item);
                }

				idx_obj_item = json_object_object_get(idx_obj, "artist");
                if(idx_obj_item)
                {
                    //printf("artist %s\n",json_object_get_string(idx_obj_item));
                    itfile->artist = json_object_get_string(idx_obj_item);
                }

				idx_obj_item = json_object_object_get(idx_obj, "album");
                if(idx_obj_item)
                {
                    //printf("album %s\n",json_object_get_string(idx_obj_item));
                    itfile->album = json_object_get_string(idx_obj_item);
                }

				idx_obj_item = json_object_object_get(idx_obj, "year");
                if(idx_obj_item)
                {
                    //printf("year %s\n",json_object_get_string(idx_obj_item));
                    itfile->year = json_object_get_string(idx_obj_item);
                }

				idx_obj_item = json_object_object_get(idx_obj, "genre");
                if(idx_obj_item)
                {
                    //printf("genre %s\n",json_object_get_string(idx_obj_item));
                    itfile->genre = json_object_get_string(idx_obj_item);
                }

				idx_obj_item = json_object_object_get(idx_obj, "copyright");
                if(idx_obj_item)
                {
                    //printf("copyright %s\n",json_object_get_string(idx_obj_item));
                    itfile->copyright = json_object_get_string(idx_obj_item);
                }
				idx_obj_item = json_object_object_get(idx_obj, "composer");
                if(idx_obj_item)
                {
                    //printf("composer %s\n",json_object_get_string(idx_obj_item));
                    itfile->composer = json_object_get_string(idx_obj_item);
                }
				idx_obj_item = json_object_object_get(idx_obj, "coverURL");
                if(idx_obj_item)
                {
                    //printf("coverURL %s\n",json_object_get_string(idx_obj_item));
                    itfile->coverURL = json_object_get_string(idx_obj_item);
                }

                addfileitem(dirstr,itfile);


            }

//            memcpy(deviceuid,json_object_get_string(idx_obj_item),strlen(json_object_get_string(idx_obj_item)));
        }
        json_object_put(para_obj);
        //释放解析json到下一次循环
    }
    printfdiritem(0);
    //getchar();
    printffileitem(dirstr,0);
    std::list<DLNADirList*>::iterator it;
    it = dirlist.begin();
    //printf("printf all dir item info :\n");
    while (it != dirlist.end()) {
        if(!strcmp((*it)->parentID.c_str(),parentID))
        {
            scandir((char *)(*it)->objectID.c_str());
        }
        it++;
    }

    /*    struct json_object* tArray = json_object_object_get(obj, "satIDList");
    int tArrayLen = json_object_array_length(tArray);
    if (tSatNum > tArrayLen)
        tSatNum = tArrayLen;

    std::vector<int> tTpsKeys;
    for (int i = 0; i < tSatNum; ++i) {
        std::vector<int> tKeys = getDvbManager().getTransponderKeys(json_object_get_int(json_object_array_get_idx(tArray, i)));
        for (std::size_t j = 0; j < tKeys.size(); ++j)
    */
}
void DLNARootList::adddiritem(DLNADirList *itdir )
{
    if (!itdir)
        return ;
    dirlist.push_back(itdir);
//    total_file;
    total_dir++;
    total_all++;
    return ;
}
void DLNARootList::addfileitem(char *dirstr,DLNAFileList *itfile )
{
    if (!itfile || !dirstr)
        return ;

    std::list<DLNADirList*>::iterator it;
    std::list<DLNAAudioFileartistList *>::iterator    artistit;
    std::list<DLNAAudioFilealbumList *>::iterator    albumit;

    it = dirlist.begin();

    //printf("printf all dir item info :\n");
    while (it != dirlist.end()) {
        if (!strcmp((*it)->objectID.c_str(),dirstr)) {
            (*it)->filelist.push_back(itfile);
            if (!strcmp(itfile->classID.c_str(),"1")) {
                (*it)->pmun++;
                ptotal_file++;
                (*it)->Isp = 1;
            }
            else  if (!strcmp(itfile->classID.c_str(),"2")) {
                (*it)->amun++;
                atotal_file++;
                (*it)->Isa = 1;

                artistit = artistList.begin();
                while(artistit != artistList.end()) {
                    if(strcmp(itfile->artist.c_str(),(*artistit)->artist.c_str()) == 0){
                        (*artistit)->mun++;
                        break;
                    	}
			artistit++;
                }
                if(artistit == artistList.end()) {
                    DLNAAudioFileartistList *itAudioFile = new DLNAAudioFileartistList;
                    itAudioFile->artist = itfile->artist;
                    itAudioFile->mun++;
                    //(*artistit)->artist = (*albumit)->artist;
                    //(*artistit)->mun++;
                    artistList.push_back(itAudioFile);

                }

                albumit = albumList.begin();
                while(albumit != albumList.end()) {
                    if(strcmp(itfile->album.c_str(),(*albumit)->album.c_str()) == 0){
                        (*albumit)->mun++;
                        break;
                    	}
			albumit++;
                }
                if(albumit == albumList.end()) {
                    DLNAAudioFilealbumList *itAudioFile = new DLNAAudioFilealbumList;
                    itAudioFile->album = itfile->album;
                    itAudioFile->mun++;
                    //(*artistit)->artist = (*albumit)->artist;
                    //(*artistit)->mun++;
                    albumList.push_back(itAudioFile);

                }


            }
            else  if (!strcmp(itfile->classID.c_str(),"3")) {
                (*it)->vmun++;
                vtotal_file++;
                (*it)->Isv = 1;
            }
            total_file++;
            total_all++;
            break;
        }
        it++;
    }
    return ;
}

void DLNARootList::printfdiritem(int detail)
{
    if (!detail) {
        //printf("total_dir = %d,total_file =%d,total_all = %d\n",total_dir,total_file,total_all);
        //printf("vtotal_file = %d,atotal_file =%d,ptotal_file = %d\n",vtotal_file,atotal_file,ptotal_file);
        return;
    }
    std::list<DLNADirList*>::iterator it;
    it = dirlist.begin();
    DLNA_LOG("printf all dir item info :\n");
    while (it != dirlist.end()) {
        DLNA_LOG("objectID %s\t",(*it)->objectID.c_str());
        DLNA_LOG("parentID %s\t",(*it)->parentID.c_str());
        DLNA_LOG("filename %s\t",(*it)->filename.c_str());
        DLNA_LOG("classID %s\t",(*it)->classID.c_str());
        //printf("dirname %s\t",(*it)->dirname.c_str());
        DLNA_LOG("\n");
        it++;
    }
    DLNA_LOG("total_dir = %d,total_file =%d,total_all = %d\n",total_dir,total_file,total_all);
    DLNA_LOG("printf all dir item info end\n");
    return;
}

void DLNARootList::printffileitem(char *dirstr,int detail)
{
    if(!dirstr || !detail) {
        return;
    }
    std::list<DLNADirList*>::iterator it;
    it = dirlist.begin();
    DLNA_LOG("printf dir %s file item info :\n",dirstr);
    while ( it != dirlist.end()) {
        if(!strcmp((*it)->objectID.c_str(),dirstr))
        {
            std::list<DLNAFileList*>::iterator itfile;
            itfile = (*it)->filelist.begin();
            while (itfile != (*it)->filelist.end()) {
                DLNA_LOG("filename %s\t",(*itfile)->filename.c_str());
                DLNA_LOG("filepath %s\t",(*itfile)->filepath.c_str());
                DLNA_LOG("objectID %s\t",(*itfile)->objectID.c_str());
                DLNA_LOG("classID %s\t",(*itfile)->classID.c_str());
                DLNA_LOG("size %s\t",(*itfile)->size.c_str());
                DLNA_LOG("bitrate %s\t",(*itfile)->bitrate.c_str());
                DLNA_LOG("duration %d\t",(*itfile)->duration);
                DLNA_LOG("protocolInfo %s\t",(*itfile)->protocolInfo.c_str());
                DLNA_LOG("\n");
                itfile++;
            }
            break;
        }
        it++;
    }
    DLNA_LOG("printf dir %s file item info end\n",dirstr);

    return ;
}
void DLNARootList::GetDmsDirectory(char *deviceID,int type,int index,int count,char *out,int len)
{

    std::list<DLNADirList*>::iterator it;
    int i = 0;
    int jsonlen = 0,strtotlen = 0;
    it = dirlist.begin();
    if ( len< 100 || index < 0) {
        return;
    }
    if(lastindex == 0) {
    }
    while (it != dirlist.end() && i < index) {//&& i < lastindex) {
        if ( (gettype == 3 && (*it)->vmun > 0) || (gettype == 2 && (*it)->amun > 0) || (gettype == 1 && (*it)->pmun > 0)){
            i++;
        }
        it++;
    }
    if(it == dirlist.end() )
        return;
    json_object *my_array = NULL, *new_object = NULL;
    my_array = json_object_new_array();
    if (!my_array) {
        DmsFillResult(my_array, total_dir, 0, out);
    }
    i = 0;
    while (it != dirlist.end() && i < count) {
        if ( (gettype == 3 && (*it)->vmun > 0) || (gettype == 2 && (*it)->amun > 0) || (gettype == 1 && (*it)->pmun > 0)){
            new_object = json_object_new_object();
            json_object_object_add(new_object, "objectID", json_object_new_string( (*it)->objectID.c_str() ));
            json_object_object_add(new_object, "parentID", json_object_new_string( (*it)->parentID.c_str() ));
            json_object_object_add(new_object, "filename", json_object_new_string( (*it)->filename.c_str() ));
            json_object_object_add(new_object, "classID", json_object_new_string( (*it)->classID.c_str() ));
            strtotlen = strlen(json_object_to_json_string(new_object));
            if ( jsonlen + strtotlen > len -100) {

                json_object_put(new_object);
                break;
            }
            jsonlen += strtotlen;
            json_object_array_add(my_array, new_object);
            i++;

        }
        lastindex++;
        it++;
    }
    DmsFillResult(my_array, total_dir, i, out);
    //json_object_put(my_array);
}

void DLNARootList::GetDmsFile(char *deviceID,int type,int index,int count,char *out,int len)
{
    std::list<DLNADirList*>::iterator it;
    std::list<DLNAFileList*>::iterator itfile;

    int i = 0, total = 0;
    int jsonlen = 0,strtotlen = 0;
    it = dirlist.begin();
     if ( len< 100 || index < 0) {
        return;
    }
    /*
    if (3 == gettype) {
        total = vtotal_file;
    }
    else if (2 == gettype) {
        total = atotal_file;
    }
    else if (1 == gettype) {
        total = ptotal_file;
    }*/
    if(lastindex == 0) {
    }
    while ( it != dirlist.end()  && i < index ) {
        itfile = (*it)->filelist.begin();
        while ( itfile != (*it)->filelist.end() && i < index ) {
            if ( (gettype == atoi( (*itfile)->classID.c_str() ) ) ) {
                i++;
            }
            itfile++;
        }
        if( i < index )
            it++;
    }
    if(it == dirlist.end() )
        return;

    json_object *my_array = NULL, *new_object = NULL;
    my_array = json_object_new_array();
    if (!my_array) {
        DmsFillResult(my_array, total, 0, out);
    }
    i = 0;
    while (it != dirlist.end() && i < count) {
        //if( (gettype == 1 && (*it)->Isp > 0) || (gettype == 2 && (*it)->Isa > 0)  || (gettype == 3 && (*it)->Isv > 0))
        //{
            itfile = (*it)->filelist.begin();
            while ( itfile != (*it)->filelist.end() && i < count) {
                if ( (gettype == atoi( (*itfile)->classID.c_str() ) ) ) {// || (gettype == 2 && (*it)->amun > 0) || (gettype == 1 && (*it)->pmun > 0)){
                    new_object = json_object_new_object();
                    json_object_object_add(new_object, "objectID", json_object_new_string( (*itfile)->objectID.c_str() ));
                    json_object_object_add(new_object, "filename", json_object_new_string( (*itfile)->filename.c_str() ));
                    json_object_object_add(new_object, "filepath", json_object_new_string( (*itfile)->filepath.c_str() ));
                    json_object_object_add(new_object, "classID", json_object_new_string( (*itfile)->classID.c_str() ));
                    json_object_object_add(new_object, "size", json_object_new_string( (*itfile)->size.c_str() ));
                    json_object_object_add(new_object, "bitrate", json_object_new_string( (*itfile)->bitrate.c_str() ));
                    json_object_object_add(new_object, "protocolInfo", json_object_new_string( (*itfile)->protocolInfo.c_str() ));
                    json_object_object_add(new_object, "duration", json_object_new_int( (*itfile)->duration ));
                    json_object_object_add(new_object, "resolution", json_object_new_string( (*itfile)->resolution.c_str() ));
                    json_object_object_add(new_object, "colorDepth", json_object_new_string( (*itfile)->colorDepth.c_str() ));
                    json_object_object_add(new_object, "contentSource", json_object_new_string( (*itfile)->contentSource.c_str() ));
                    json_object_object_add(new_object, "date", json_object_new_string( (*itfile)->date.c_str() ));
                    json_object_object_add(new_object, "artist", json_object_new_string( (*itfile)->artist.c_str() ));
                    json_object_object_add(new_object, "album", json_object_new_string( (*itfile)->album.c_str() ));
                    json_object_object_add(new_object, "year", json_object_new_string( (*itfile)->year.c_str() ));
                    json_object_object_add(new_object, "genre", json_object_new_string( (*itfile)->genre.c_str() ));
                    json_object_object_add(new_object, "copyright", json_object_new_string( (*itfile)->copyright.c_str() ));
                    json_object_object_add(new_object, "composer", json_object_new_string( (*itfile)->composer.c_str() ));
                    json_object_object_add(new_object, "coverURL", json_object_new_string( (*itfile)->coverURL.c_str() ));


                    strtotlen = strlen(json_object_to_json_string(new_object));
                    if ( jsonlen + strtotlen > len -100) {

                        json_object_put(new_object);
                        break;
                    }
                    jsonlen += strtotlen;
                    json_object_array_add(my_array, new_object);
                    i++;
                }
                lastindex++;
                itfile++;
            }
        //}
        it++;
    }
    DmsFillResult(my_array, total, i, out);

}

void DLNARootList::GetDmsAudioFile(char *deviceID,int type,int index,int count,char *out,int len)
{
    std::list<DLNADirList*>::iterator it;
    std::list<DLNAFileList*>::iterator itfile;

    int i = 0, total = 0;
    int jsonlen = 0,strtotlen = 0;
    it = dirlist.begin();
     if ( len< 100 || index < 0) {
        return;
    }
    if(lastindex == 0 ) {
    }
    while ( it != dirlist.end()  && i < index ) {
        itfile = (*it)->filelist.begin();
        while ( itfile != (*it)->filelist.end() && i < index ) {
            if ( (2 == atoi( (*itfile)->classID.c_str() ) ) ) {
                if ( (FilterAudiotype == 1 && 0 == strcmp((*itfile)->artist.c_str(),Audiokeyword))
                    ||(FilterAudiotype == 2 && 0 == strcmp((*itfile)->album.c_str(),Audiokeyword))) {
                    i++;
                }
            }
            itfile++;
        }
        if( i < index)
            it++;
    }
    if(it == dirlist.end() )
        return;

    json_object *my_array = NULL, *new_object = NULL;
    my_array = json_object_new_array();
    if (!my_array) {
        DmsFillResult(my_array, total, 0, out);
    }
    i = 0;
    while (it != dirlist.end() && i < count) {
        //if( (gettype == 1 && (*it)->Isp > 0) || (gettype == 2 && (*it)->Isa > 0)  || (gettype == 3 && (*it)->Isv > 0))
        //{
            itfile = (*it)->filelist.begin();
            while ( itfile != (*it)->filelist.end() && i < count) {
                if ( (2 == atoi( (*itfile)->classID.c_str() ) ) ) {// || (gettype == 2 && (*it)->amun > 0) || (gettype == 1 && (*it)->pmun > 0)){
                    if ( (FilterAudiotype == 1 && 0 == strcmp((*itfile)->artist.c_str(),Audiokeyword))
                        ||(FilterAudiotype == 2 && 0 == strcmp((*itfile)->album.c_str(),Audiokeyword))) {
	                    new_object = json_object_new_object();
	                    json_object_object_add(new_object, "objectID", json_object_new_string( (*itfile)->objectID.c_str() ));
	                    json_object_object_add(new_object, "filename", json_object_new_string( (*itfile)->filename.c_str() ));
	                    json_object_object_add(new_object, "filepath", json_object_new_string( (*itfile)->filepath.c_str() ));
	                    json_object_object_add(new_object, "classID", json_object_new_string( (*itfile)->classID.c_str() ));
	                    json_object_object_add(new_object, "size", json_object_new_string( (*itfile)->size.c_str() ));
	                    json_object_object_add(new_object, "bitrate", json_object_new_string( (*itfile)->bitrate.c_str() ));
	                    json_object_object_add(new_object, "protocolInfo", json_object_new_string( (*itfile)->protocolInfo.c_str() ));
	                    json_object_object_add(new_object, "duration", json_object_new_int( (*itfile)->duration ));
	                    json_object_object_add(new_object, "resolution", json_object_new_string( (*itfile)->resolution.c_str() ));
	                    json_object_object_add(new_object, "colorDepth", json_object_new_string( (*itfile)->colorDepth.c_str() ));
	                    json_object_object_add(new_object, "contentSource", json_object_new_string( (*itfile)->contentSource.c_str() ));
	                    json_object_object_add(new_object, "date", json_object_new_string( (*itfile)->date.c_str() ));
	                    json_object_object_add(new_object, "artist", json_object_new_string( (*itfile)->artist.c_str() ));
	                    json_object_object_add(new_object, "album", json_object_new_string( (*itfile)->album.c_str() ));
	                    json_object_object_add(new_object, "year", json_object_new_string( (*itfile)->year.c_str() ));
	                    json_object_object_add(new_object, "genre", json_object_new_string( (*itfile)->genre.c_str() ));
	                    json_object_object_add(new_object, "copyright", json_object_new_string( (*itfile)->copyright.c_str() ));
	                    json_object_object_add(new_object, "composer", json_object_new_string( (*itfile)->composer.c_str() ));
	                    json_object_object_add(new_object, "coverURL", json_object_new_string( (*itfile)->coverURL.c_str() ));


	                    strtotlen = strlen(json_object_to_json_string(new_object));
	                    if ( jsonlen + strtotlen > len -100) {

	                        json_object_put(new_object);
	                        break;
	                    }
	                    jsonlen += strtotlen;
	                    json_object_array_add(my_array, new_object);
	                    i++;
                    }
                }
                lastindex++;
                itfile++;
            }
        //}
        it++;
    }
    DmsFillResult(my_array, total, i, out);

}

void DLNARootList::GetDmsDirectoryFile(char *deviceID,char *DirObjectID,int type,int index,int count,char *out,int len)
{
    std::list<DLNADirList*>::iterator it;
    int i = 0;
    int jsonlen = 0,strtotlen = 0;
    it = dirlist.begin();
     if ( len< 100 || index < 0) {
        return;
    }
    if(lastindex == 0 ) {
    }
    while (it != dirlist.end() ) {
        if (!strcmp((*it)->objectID.c_str(),DirectoryObjectID)) {
            break;
        }
        it++;
    }//find dir
    if(it == dirlist.end() )
        return;
    std::list<DLNAFileList*>::iterator itfile;
    itfile = (*it)->filelist.begin();
    while ( itfile != (*it)->filelist.end() && i < index){
        if ( (gettype == atoi( (*itfile)->classID.c_str() ) ) ) {
            i++;
        }
        itfile++;
    }//find last file
    if(itfile == (*it)->filelist.end() )
        return;
    json_object *my_array = NULL, *new_object = NULL;
    my_array = json_object_new_array();
    if (!my_array) {
        DmsFillResult(my_array, 0, 0, out);
    }
    i = 0;
    while ( itfile != (*it)->filelist.end() && i < count) {
        if ( (gettype == atoi( (*itfile)->classID.c_str() ) ) ) {// || (gettype == 2 && (*it)->amun > 0) || (gettype == 1 && (*it)->pmun > 0)){
            new_object = json_object_new_object();
            json_object_object_add(new_object, "objectID", json_object_new_string( (*itfile)->objectID.c_str() ));
            json_object_object_add(new_object, "filename", json_object_new_string( (*itfile)->filename.c_str() ));
            json_object_object_add(new_object, "filepath", json_object_new_string( (*itfile)->filepath.c_str() ));
            json_object_object_add(new_object, "classID", json_object_new_string( (*itfile)->classID.c_str() ));
            json_object_object_add(new_object, "size", json_object_new_string( (*itfile)->size.c_str() ));
            json_object_object_add(new_object, "bitrate", json_object_new_string( (*itfile)->bitrate.c_str() ));
            json_object_object_add(new_object, "protocolInfo", json_object_new_string( (*itfile)->protocolInfo.c_str() ));
            json_object_object_add(new_object, "duration", json_object_new_int( (*itfile)->duration ));
            json_object_object_add(new_object, "resolution", json_object_new_string( (*itfile)->resolution.c_str() ));
            json_object_object_add(new_object, "colorDepth", json_object_new_string( (*itfile)->colorDepth.c_str() ));
            json_object_object_add(new_object, "contentSource", json_object_new_string( (*itfile)->contentSource.c_str() ));
            json_object_object_add(new_object, "date", json_object_new_string( (*itfile)->date.c_str() ));
            json_object_object_add(new_object, "artist", json_object_new_string( (*itfile)->artist.c_str() ));
            json_object_object_add(new_object, "album", json_object_new_string( (*itfile)->album.c_str() ));
            json_object_object_add(new_object, "year", json_object_new_string( (*itfile)->year.c_str() ));
            json_object_object_add(new_object, "genre", json_object_new_string( (*itfile)->genre.c_str() ));
            json_object_object_add(new_object, "copyright", json_object_new_string( (*itfile)->copyright.c_str() ));
            json_object_object_add(new_object, "composer", json_object_new_string( (*itfile)->composer.c_str() ));
            json_object_object_add(new_object, "coverURL", json_object_new_string( (*itfile)->coverURL.c_str() ));


            strtotlen = strlen(json_object_to_json_string(new_object));
            if ( jsonlen + strtotlen > len -100) {
                json_object_put(new_object);
                break;
            }
            jsonlen += strtotlen;
            json_object_array_add(my_array, new_object);
            i++;
            }
        itfile++;
        lastindex++;
    }
    DmsFillResult(my_array, 0, i, out);


}
void DLNARootList::GetSearchFile(int index,int count,char *out,int len)
{

    if( out && (!search_array || len < 100) ) {
        if(len>20) {
            strcpy(out, "GetSearchFile err!");
        }
        return ;
    }
    DLNA_LOG("index %d count %d   \n",index,count);

    int json_array_len = json_object_array_length(search_array);
    int i = 0, j = 0,strtotlen = 0,jsonlen = 0;
    json_object  *my_array = NULL,*new_object = NULL,*new_object_temp = NULL;
    my_array = json_object_new_array();
    if (!my_array) {
        DmsFillResult(my_array, 0, 0, out);
    }

    //得到返回值中数组长度

    for( i = index;i < json_array_len ;i++)
    {
        new_object = json_object_array_get_idx(search_array, i);
        new_object_temp = json_object_new_object();
        if (!new_object_temp || !new_object) {
            DmsFillResult(my_array, 0, 0, out);
            break;
        }
        //json_object_object_add(new_object_temp,"");

        DLNA_LOG("new_object = %s\n",json_object_get_string(new_object));
        json_object  *idx_obj_item_1 = NULL;
        idx_obj_item_1 = json_object_object_get(new_object, "objectID");
        if(idx_obj_item_1)
        {
            DLNA_LOG("idx_obj_item_1 objectID %s \n",json_object_get_string(idx_obj_item_1));
            json_object_object_add(new_object_temp, "objectID", json_object_new_string("3$536870962$2013266000"));
        }
        DLNA_LOG("\n");
        idx_obj_item_1 = json_object_object_get(new_object, "filename");
                DLNA_LOG("\n");

        if(idx_obj_item_1)
        {
            DLNA_LOG("idx_obj_item_1 filename %s\n",json_object_get_string(idx_obj_item_1));
            json_object_object_add(new_object_temp, "filename", json_object_new_string(json_object_get_string(idx_obj_item_1)));
        }

        idx_obj_item_1 = json_object_object_get(new_object, "filepath");
        if(idx_obj_item_1)
        {
            DLNA_LOG("idx_obj_item_1 filepath %s\n",json_object_get_string(idx_obj_item_1));
            json_object_object_add(new_object_temp, "filepath", json_object_new_string(json_object_get_string(idx_obj_item_1)));
        }

        idx_obj_item_1 = json_object_object_get(new_object, "classID");
        if(idx_obj_item_1)
        {
            DLNA_LOG("idx_obj_item_1 classID %s\n",json_object_get_string(idx_obj_item_1));
            json_object_object_add(new_object_temp, "classID", json_object_new_string(json_object_get_string(idx_obj_item_1)));
        }

        idx_obj_item_1 = json_object_object_get(new_object, "size");
        if(idx_obj_item_1)
        {
            DLNA_LOG("idx_obj_item_1 size %s\n",json_object_get_string(idx_obj_item_1));
            json_object_object_add(new_object_temp, "size", json_object_new_string(json_object_get_string(idx_obj_item_1)));
        }

        idx_obj_item_1 = json_object_object_get(new_object, "bitrate");
        if(idx_obj_item_1)
        {
            DLNA_LOG("idx_obj_item_1 bitrate %s\n",json_object_get_string(idx_obj_item_1));
            json_object_object_add(new_object_temp, "bitrate", json_object_new_string(json_object_get_string(idx_obj_item_1)));
        }
        idx_obj_item_1 = json_object_object_get(new_object, "protocolInfo");
        if(idx_obj_item_1)
        {
            DLNA_LOG("idx_obj_item_1 protocolInfo %s\n",json_object_get_string(idx_obj_item_1));
            json_object_object_add(new_object_temp, "protocolInfo", json_object_new_string(json_object_get_string(idx_obj_item_1)));
        }

        idx_obj_item_1 = json_object_object_get(new_object, "duration");
        if(idx_obj_item_1)
        {
            DLNA_LOG("idx_obj_item_1 duration %s\n",json_object_get_string(idx_obj_item_1));
            json_object_object_add(new_object_temp, "duration", json_object_new_int(json_object_get_int(idx_obj_item_1)));
        }

	 idx_obj_item_1 = json_object_object_get(new_object, "resolution");
        if(idx_obj_item_1)
        {
            DLNA_LOG("idx_obj_item_1 resolution %s\n",json_object_get_string(idx_obj_item_1));
            json_object_object_add(new_object_temp, "resolution", json_object_new_string(json_object_get_string(idx_obj_item_1)));
        }

        idx_obj_item_1 = json_object_object_get(new_object, "colorDepth");
        if(idx_obj_item_1)
        {
            DLNA_LOG("idx_obj_item_1 colorDepth %s\n",json_object_get_string(idx_obj_item_1));
            json_object_object_add(new_object_temp, "colorDepth", json_object_new_string(json_object_get_string(idx_obj_item_1)));
        }

        idx_obj_item_1 = json_object_object_get(new_object, "colorDepth");
        if(idx_obj_item_1)
        {
            DLNA_LOG("idx_obj_item_1 colorDepth %s\n",json_object_get_string(idx_obj_item_1));
            json_object_object_add(new_object_temp, "colorDepth", json_object_new_string(json_object_get_string(idx_obj_item_1)));
        }

        idx_obj_item_1 = json_object_object_get(new_object, "contentSource");
        if(idx_obj_item_1)
        {
            DLNA_LOG("idx_obj_item_1 contentSource %s\n",json_object_get_string(idx_obj_item_1));
            json_object_object_add(new_object_temp, "contentSource", json_object_new_string(json_object_get_string(idx_obj_item_1)));
        }

        idx_obj_item_1 = json_object_object_get(new_object, "date");
        if(idx_obj_item_1)
        {
            DLNA_LOG("idx_obj_item_1 date %s\n",json_object_get_string(idx_obj_item_1));
            json_object_object_add(new_object_temp, "date", json_object_new_string(json_object_get_string(idx_obj_item_1)));
        }

        idx_obj_item_1 = json_object_object_get(new_object, "artist");
        if(idx_obj_item_1)
        {
            DLNA_LOG("idx_obj_item_1 artist %s\n",json_object_get_string(idx_obj_item_1));
            json_object_object_add(new_object_temp, "artist", json_object_new_string(json_object_get_string(idx_obj_item_1)));
        }

        idx_obj_item_1 = json_object_object_get(new_object, "album");
        if(idx_obj_item_1)
        {
            DLNA_LOG("idx_obj_item_1 album %s\n",json_object_get_string(idx_obj_item_1));
            json_object_object_add(new_object_temp, "album", json_object_new_string(json_object_get_string(idx_obj_item_1)));
        }

        idx_obj_item_1 = json_object_object_get(new_object, "year");
        if(idx_obj_item_1)
        {
            DLNA_LOG("idx_obj_item_1 year %s\n",json_object_get_string(idx_obj_item_1));
            json_object_object_add(new_object_temp, "year", json_object_new_string(json_object_get_string(idx_obj_item_1)));
        }

        idx_obj_item_1 = json_object_object_get(new_object, "genre");
        if(idx_obj_item_1)
        {
            DLNA_LOG("idx_obj_item_1 genre %s\n",json_object_get_string(idx_obj_item_1));
            json_object_object_add(new_object_temp, "genre", json_object_new_string(json_object_get_string(idx_obj_item_1)));
        }

        idx_obj_item_1 = json_object_object_get(new_object, "copyright");
        if(idx_obj_item_1)
        {
            DLNA_LOG("idx_obj_item_1 copyright %s\n",json_object_get_string(idx_obj_item_1));
            json_object_object_add(new_object_temp, "copyright", json_object_new_string(json_object_get_string(idx_obj_item_1)));
        }

        idx_obj_item_1 = json_object_object_get(new_object, "composer");
        if(idx_obj_item_1)
        {
            DLNA_LOG("idx_obj_item_1 composer %s\n",json_object_get_string(idx_obj_item_1));
            json_object_object_add(new_object_temp, "composer", json_object_new_string(json_object_get_string(idx_obj_item_1)));
        }

        idx_obj_item_1 = json_object_object_get(new_object, "coverURL");
        if(idx_obj_item_1)
        {
            DLNA_LOG("idx_obj_item_1 coverURL %s\n",json_object_get_string(idx_obj_item_1));
            json_object_object_add(new_object_temp, "coverURL", json_object_new_string(json_object_get_string(idx_obj_item_1)));
        }


    //json_object_new_string(json_object_to_json_string(new_object));
        strtotlen = strlen(json_object_to_json_string(new_object_temp));
        if ( jsonlen + strtotlen > len - 100) {
            break;
        }
        jsonlen += strtotlen;
        json_object_array_add(my_array, new_object_temp);
        j++;
        if(j >= count) {
            break;
        }

    }
    DmsFillResult(my_array, 0,j, out);

}
void DLNARootList:: DmsFillResult(json_object *my_array, int totnum, int num, char *value)
{
    json_object *json = json_object_new_object();
    if( json ) {
        if(my_array && num) {
            //json_object_object_add(json, "totalcount", json_object_new_int(totnum));
            json_object_object_add(json, "count", json_object_new_int(num));
            json_object_object_add(json, "fileList", my_array);
        }
        else {
            //json_object_object_add(json, "totalcount", json_object_new_int(totnum));
            json_object_object_add(json, "count", json_object_new_int(0));
        }
        strcpy(value, json_object_to_json_string(json));
        json_object_put(json);
    }
}
int DLNARootList::DmsGetCount(void)
{
    int i = 0;
    //printf("DmsGetCount\n");
    DLNA_LOG("DirectoryObjectID = %s\n",DirectoryObjectID);
    DLNA_LOG("keyword = %s\n",keyword);
    DLNA_LOG("searchtype = %d\n",searchtype);
    DLNA_LOG("directory = %d\n",directory);
    //DLNA_LOG("DirectoryObjectID = %s\n",DirectoryObjectID);
    //DLNA_LOG("DirectoryObjectID = %s\n",DirectoryObjectID);

    if(directory){//取目录
       // (*itroot)->GetDmsDirectory(deviceID,(*itroot)->gettype,index,count,out,len);
        std::list<DLNADirList*>::iterator it;
        it = dirlist.begin();
        while (it != dirlist.end()) {
            if ( (gettype == 3 && (*it)->vmun > 0) || (gettype == 2 && (*it)->amun > 0) || (gettype == 1 && (*it)->pmun > 0)){
                i++;
                //printf("DmsGetCount %d \n",i);

            }
            it++;
        }
    }
    else if( searchtype && (0 != strcmp(keyword,"NULL")) ) {
        i = SearchFile();
    }
    else if(0 == strcmp(DirectoryObjectID,"NULL")){//取文件
        //(*itroot)->GetDmsFile(deviceID,(*itroot)->gettype,index,count,out,len);
        std::list<DLNADirList*>::iterator it;
        it = dirlist.begin();
        while (it != dirlist.end()) {
            if ( gettype == 3 && (*it)->vmun > 0){
                i += (*it)->vmun;
            }
            else if (gettype == 2 && (*it)->amun > 0) {
                i += (*it)->amun;
            }
            else if (gettype == 1 && (*it)->pmun > 0) {
                i += (*it)->pmun;
            }
            it++;

        }

    }
    else {//取目录下文件
       // (*itroot)->GetDmsDirectoryFile(deviceID,(*itroot)->DirectoryObjectID,(*itroot)->gettype,index,count,out,len);
        std::list<DLNADirList*>::iterator it;
        it = dirlist.begin();
        while (it != dirlist.end() ) {
        if (!strcmp((*it)->objectID.c_str(),DirectoryObjectID)) {
            if ( gettype == 3 && (*it)->vmun > 0){
                i += (*it)->vmun;
            }
            else if (gettype == 2 && (*it)->amun > 0) {
                i += (*it)->amun;
            }
            else if (gettype == 1 && (*it)->pmun > 0) {
                i += (*it)->pmun;
            }
            break;
        }
        it++;
    }//find dir

    }
    return i;
}
int DLNARootList::IsNeedNile(char *name)
{
    char pout[128] = {0};
    int len = sizeof(pout);
    DLNA_LOG("name%s keyword%s \n",name,keyword);
    if(strstr(name,keyword)) {
        DLNA_LOG("1 name%s keyword%s \n",name,keyword);
        return 1;
    }    DLNA_LOG("\n");

    getAcronymFromUtf8((unsigned char *)name, sizeof(keyword), pout, &len);
    DLNA_LOG("\n");
    char *p = NULL;
    p = (char *)strstr(pout,"|");
    DLNA_LOG("\n");
    if (p) {
        *(p+1) = '\0';
    }
    DLNA_LOG("pout =%s\n",pout);
    if(strstr(pout,keyword)) {
        DLNA_LOG("1 name%s keyword%s \n",name,keyword);
        return 1;
    }
    DLNA_LOG("0 name%s keyword%s \n",name,keyword);

    return 0;
}

int DLNARootList::SearchFile(void)
{
    std::list<DLNADirList*>::iterator it;
    std::list<DLNAFileList*>::iterator itfile;
    json_object *new_object = NULL;
	DLNA_LOG("\n");
    json_object_put(search_array);
    search_array = json_object_new_array();
    if(!search_array) {
        return 0;
    }
    int i = 0, total = 0;
    int jsonlen = 0,strtotlen = 0;
    it = dirlist.begin();

    while (it != dirlist.end() ) {
            itfile = (*it)->filelist.begin();
            while ( itfile != (*it)->filelist.end()) {
                if ( IsNeedNile((char *)(*itfile)->filename.c_str()) ) {// || (gettype == 2 && (*it)->amun > 0) || (gettype == 1 && (*it)->pmun > 0)){
                    new_object = json_object_new_object();
                    json_object_object_add(new_object, "objectID", json_object_new_string( (*itfile)->objectID.c_str() ));
                    json_object_object_add(new_object, "filename", json_object_new_string( (*itfile)->filename.c_str() ));
                    json_object_object_add(new_object, "filepath", json_object_new_string( (*itfile)->filepath.c_str() ));
                    json_object_object_add(new_object, "classID", json_object_new_string( (*itfile)->classID.c_str() ));
                    json_object_object_add(new_object, "size", json_object_new_string( (*itfile)->size.c_str() ));
                    json_object_object_add(new_object, "bitrate", json_object_new_string( (*itfile)->bitrate.c_str() ));
                    json_object_object_add(new_object, "protocolInfo", json_object_new_string( (*itfile)->protocolInfo.c_str() ));
                    json_object_object_add(new_object, "duration", json_object_new_int( (*itfile)->duration ));
                    json_object_object_add(new_object, "resolution", json_object_new_string( (*itfile)->resolution.c_str() ));
                    json_object_object_add(new_object, "colorDepth", json_object_new_string( (*itfile)->colorDepth.c_str() ));
                    json_object_object_add(new_object, "contentSource", json_object_new_string( (*itfile)->contentSource.c_str() ));
                    json_object_object_add(new_object, "date", json_object_new_string( (*itfile)->date.c_str() ));
                    json_object_object_add(new_object, "artist", json_object_new_string( (*itfile)->artist.c_str() ));
                    json_object_object_add(new_object, "album", json_object_new_string( (*itfile)->album.c_str() ));
                    json_object_object_add(new_object, "year", json_object_new_string( (*itfile)->year.c_str() ));
                    json_object_object_add(new_object, "genre", json_object_new_string( (*itfile)->genre.c_str() ));
                    json_object_object_add(new_object, "copyright", json_object_new_string( (*itfile)->copyright.c_str() ));
                    json_object_object_add(new_object, "composer", json_object_new_string( (*itfile)->composer.c_str() ));
                    json_object_object_add(new_object, "coverURL", json_object_new_string( (*itfile)->coverURL.c_str() ));


					json_object_array_add(search_array, new_object);
                    i++;
                }
                //lastindex++;
                itfile++;
            }
        //}
        it++;
    }
    //DmsFillResult(my_array, total, i, out);
    return i;
}
DLNARootList::DLNARootList()
    :m_ScanStatus(Scan_NotStart)
    ,vtotal_file(0)
    ,atotal_file(0)
    ,ptotal_file(0)
    ,total_file(0)
    ,total_dir(0)
    ,total_all(0)
    ,stopflag(0)
    ,gettype(0)
    ,directory(0)
    ,lastindex(0)
    ,searchtype(0)
    ,FilterAudiotype(0)
{
    memset(dmsname,0,sizeof(dmsname));
    memset(DirectoryObjectID,0,sizeof(DirectoryObjectID));
    memset(keyword,0,sizeof(keyword));
    memset(Audiokeyword,0,sizeof(Audiokeyword));

    search_array = NULL;
}
DLNARootList::~DLNARootList()
{

    std::list<DLNADirList*>::iterator it;
    it = dirlist.begin();
    while ( it != dirlist.end()) {
        delete *it;
        it = dirlist.erase(it) ;
    }

    std::list<DLNAAudioFileartistList *>::iterator    artistit;
    artistit = artistList.begin();
    while ( artistit != artistList.end()) {
        delete *artistit;
        artistit = artistList.erase(artistit) ;
    }

    std::list<DLNAAudioFilealbumList *>::iterator    albumit;
    albumit = albumList.begin();
    while ( albumit != albumList.end()) {
        delete *albumit;
        albumit = albumList.erase(albumit) ;
    }

    json_object_put(search_array);

}
DLNADirList::DLNADirList()
    :vmun(0)
    ,amun(0)
    ,pmun(0)
    ,Isp(0)
    ,Isa(0)
    ,Isv(0)

{
    /*
    memset(objectID,0,sizeof(objectID));
    memset(parentID,0,sizeof(parentID));
    memset(filename,0,sizeof(filename));
    memset(classID,0,sizeof(classID));
    */
}
DLNADirList::~DLNADirList()
{
    std::list<DLNAFileList*>::iterator itfile;
    itfile = filelist.begin();
    while (itfile != filelist.end()) {
        delete *itfile;
        itfile = filelist.erase(itfile) ;
    }

}
DLNAFileList::DLNAFileList()
    :duration(0)
{
}
DLNAFileList::~DLNAFileList()
{
}

DLNAAudioFileartistList::DLNAAudioFileartistList()
    :mun(0)
{

}
DLNAAudioFileartistList::~DLNAAudioFileartistList()
{

}

DLNAAudioFilealbumList::DLNAAudioFilealbumList()
    :mun(0)
{

}
DLNAAudioFilealbumList::~DLNAAudioFilealbumList()
{

}



}

