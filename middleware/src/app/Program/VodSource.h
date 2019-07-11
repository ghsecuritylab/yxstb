#ifndef _VodSource_H_
#define _VodSource_H_

#include <string>

#ifdef __cplusplus

namespace Hippo {

class VodSource {
public:
    VodSource();
    ~VodSource();

    std::string& GetUrl(){return m_url;};
    void SetUrl(std::string& CStr){ m_url = CStr;};
    std::string& GetMediaCode(){return m_mediaCode;};
    void SetMediaCode(std::string& CStr){m_mediaCode = CStr;};
    std::string& GetEntryID(){return m_entryID;};
    void SetEntryID(std::string& CStr){m_entryID = CStr;};
    std::string& GetStartTime(){return m_startTime;};
    void SetStartTime(std::string& CStr){m_startTime = CStr;};
    std::string& GetEndTime(){return m_endTime;};
    void SetEndTime(std::string& CStr){m_endTime = CStr;};

    int GetMediaType(){return m_mediaType;};
    void SetMediaType(int value){m_mediaType = value;};
    int GetAudioType(){return m_audioType;};
    void SetAudioType(int value){m_audioType = value;};
    int GetVideoType(){return m_videoType;};
    void SetVideoType(int value){m_videoType = value;};
    int GetStreamType(){return m_streamType;};
    void SetStreamType(int value){m_streamType = value;};
    int GetDrmType(){return m_drmType;};
    void SetDrmType(int value){m_drmType = value;};
    int GetFingerPrint(){return m_fingerPrint;};
    void SetFingerPrint(int value){m_fingerPrint = value;};
    int GetCopyProtection(){return m_copyProtection;};
    void SetCopyProtection(int value){m_copyProtection = value;};
    int GetAllowTrickmode(){return m_allowTrickmode;};
    void SetAllowTrickmode(int value){m_allowTrickmode = value;};
    int GetInsertTime(){return m_insertTime;};
    void SetInsertTime(int value){m_insertTime = value;};
    int GetIsPostive(){return m_isPostive;};
    void SetIsPostive(int value){m_isPostive = value;};
    int GetBandwidth(){return m_bandwidth;};
    void SetBandwidth(int value){m_bandwidth = value;};

protected:

    std::string m_url;
    std::string m_mediaCode;
    std::string m_entryID;
    std::string m_startTime;
    std::string m_endTime;
    int m_mediaType;
    int m_audioType;
    int m_videoType;
    int m_streamType;
    int m_drmType;
    int m_fingerPrint;
    int m_copyProtection;
    int m_allowTrickmode;
    int m_insertTime;
    int m_isPostive;
    int m_bandwidth;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _ProgramInfo_H_
