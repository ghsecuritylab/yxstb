#include "BrowserPlayerMsgCode.h"
#include "BrowserPlayerReporter.h"

#define CODE_BASE 1000

namespace Hippo {
    
PlayerCode playerCodeArray[] = {
{Event_MediaError, STRM_MSG_OPEN_ERROR, 0, 0, 104001, "Play Failed", "Error", Print_None, ""},	
{Event_MediaError, STRM_MSG_OPEN_ERROR, RTSP_CODE_Bad_Request, 400, 104050, "Bad Request", "Error", Print_None, ""},
{Event_MediaError, STRM_MSG_OPEN_ERROR, RTSP_CODE_Unauthorized, 401, 104003, "Unauthorized", "Error", Print_None, ""},
{Event_MediaError, STRM_MSG_OPEN_ERROR, RTSP_CODE_Forbidden, 403, 104051, "Forbidden", "Error", Print_None, ""},
{Event_MediaError, STRM_MSG_OPEN_ERROR, RTSP_CODE_Not_Found, 404, 104002, "Not Found", "Error", Print_None, ""},    
{Event_MediaError, STRM_MSG_OPEN_ERROR, RTSP_CODE_Method_Not_Allowed, 405, 104008, "Method Not Allowed", "Error", Print_None, ""},
{Event_MediaError, STRM_MSG_OPEN_ERROR, RTSP_CODE_Not_Acceptable, 406, 104052, "Not Acceptable", "Error", Print_None, ""},
{Event_MediaError, STRM_MSG_OPEN_ERROR, RTSP_CODE_Unsupported_Media_Type, 415, 104005, "Unsupported MediaType", "Error", Print_None, ""},
{Event_MediaError, STRM_MSG_OPEN_ERROR, RTSP_CODE_Parameter_Not_Understood, 451, 104006, "Parameter Not Understood", "Error", Print_None, ""},
{Event_MediaError, STRM_MSG_OPEN_ERROR, RTSP_CODE_Session_Not_Found, 454, 104010, "Session Not Found", "Error", Print_None, ""},
{Event_MediaError, STRM_MSG_OPEN_ERROR, RTSP_CODE_Method_Not_Valid_In_This_State, 455, 104053, "Method Not Valid In This State", "Error", Print_None, ""},
{Event_MediaError, STRM_MSG_OPEN_ERROR, RTSP_CODE_Unsupported_Transport, 461, 104011, "Unsupported Transport", "Error", Print_None, ""},
{Event_MediaError, STRM_MSG_OPEN_ERROR, RTSP_CODE_Internal_Server_Error, 500, 104016,"Internal Server Error", "Error", Print_None, ""},
{Event_MediaError, STRM_MSG_OPEN_ERROR, RTSP_CODE_Not_Implemented, 501, 104054, "Not Implemented", "Error", Print_None, ""},
{Event_MediaError, STRM_MSG_OPEN_ERROR, RTSP_CODE_Service_Unavailable, 503, 104012, "Service Unavailable", "Error", Print_None, ""},
{Event_MediaError, STRM_MSG_OPEN_ERROR, RTSP_CODE_Version_Not_Supported, 505, 104013, "RTSP Version Not Supported", "Error", Print_None, ""},    
{Event_MediaError, STRM_MSG_OPEN_ERROR, RTSP_CODE_Option_Not_Supported, 551, 104014, "Option Not Support", "Error", Print_None, ""},    
{Event_MediaError, STRM_MSG_OPEN_ERROR, RTSP_CODE_URL_FORMAT_Error, 10081, 104001, "Can not find the relevant program.", "Error", Print_ErrUrl, "Description: The EPG invokes playback, but the channel or program URL delivered to the STB is blank or in an incorrect format. Error URL:"},
{Event_MediaError, STRM_MSG_OPEN_ERROR, RTSP_CODE_Connect_Error, 10088, 104001, "Program broadcast temporarily abnormal.", "Error", Print_ErrUrl, "Description: The STB fails to setup a connation to the Media Server (socket connection fails). Hms URL:"},
{Event_MediaError, STRM_MSG_OPEN_ERROR, RTSP_CODE_Connect_Timeout, 10088, 104015, "Program broadcast temporarily abnormal.", "Error", Print_ErrUrl, "Description: The STB fails to setup a connation to the Media Server (socket connection fails). Hms URL:"},
{Event_MediaError, STRM_MSG_OPEN_ERROR, RTSP_CODE_Session_Timeout, 10089, 104001, "Program broadcast temporarily abnormal.", "Error", Print_All, "Description: The STB is properly connected to the Media Server but singling interaction times out, causing that accessing the stream receiving and playback phase fails. Hms URL:"},
{Event_MediaError, STRM_MSG_OPEN_ERROR, RTSP_CODE_Play_Timeout, 10090, 104001, "Program broadcast temporarily abnormal.", "Error", Print_All, "Description: The STB sends a playback control command, however, the Media Server sends no response due to a timeout. Hms URL:"},  
{Event_MediaError, STRM_MSG_OPEN_ERROR, RTSP_CODE_Play_Error, 10085, 104001, "Program broadcast temporarily abnormal.", "Error", Print_All, "Description: The STB fails to send a playback control command. Hms URL:"},  
{Event_MediaError, STRM_MSG_OPEN_ERROR, RTSP_CODE_Parse_Error, 10084, 104001, "Program broadcast temporarily abnormal.", "Error", Print_ErrCmd, "Description: During the negotiation phase before media stream playback, the STB cannot parse the singling sent back by the Media Server. Error Command:"},    
{Event_MediaError, STRM_MSG_OPEN_ERROR, RTSP_CODE_Socket_Start_Error, 10082, 104001, "Program broadcast temporarily abnormal.", "Error", Print_ErrUrl, "Description: When playback starts, the interaction with the Media Server is normal but the STB does not receive media streams. Hms URL:"},
{Event_MediaError, STRM_MSG_OPEN_ERROR, RTSP_CODE_Socket_Playing_Error, 10086, 104001, "Program broadcast temporarily abnormal.", "Error", Print_ErrUrl, "Description: The STB is disconnected from the Media Server during playback, causing media stream interruption. Hms URL:"},        
{Event_MediaError, STRM_MSG_OPEN_ERROR, RTSP_CODE_server_Error, 10087, 104001, "Program broadcast temporarily abnormal.", "Error", Print_ErrUrl, "Description: The Media Server disables the RTSP connection but media streams are not interrupted and program playback continues. Hms URL:"},
{Event_MediaError, STRM_MSG_OPEN_ERROR, PVR_CODE_NOT_FOUND, 402, 104020, "Payment Required", "Error", Print_None, ""},
{Event_MediaError, STRM_MSG_OPEN_ERROR, PVR_CODE_FILE_DAMAGE, 402, 104021, "Payment Required", "Error", Print_None, ""},
{Event_MediaError, STRM_MSG_OPEN_ERROR, PVR_CODE_BUFFER_FAILED, 402, 104023, "Payment Required", "Error", Print_None, ""},
{Event_MediaError, STRM_MSG_OPEN_ERROR, 402, 402, 104001, "Payment Required", "", Print_None, ""},
{Event_MediaError, STRM_MSG_OPEN_ERROR, 407, 407, 104001, "Proxy Authentication Required", "", Print_None, ""},
{Event_MediaError, STRM_MSG_OPEN_ERROR, 408, 408, 104001, "Request Timeout", "", Print_None, ""},
{Event_MediaError, STRM_MSG_OPEN_ERROR, 410, 410, 104001, "Gone", "", Print_None, ""},
{Event_MediaError, STRM_MSG_OPEN_ERROR, 411, 411, 104001, "Length Required", "", Print_None, ""},
{Event_MediaError, STRM_MSG_OPEN_ERROR, 412, 412, 104001, "Precondition Failed", "", Print_None, ""},
{Event_MediaError, STRM_MSG_OPEN_ERROR, 413, 413, 104001, "Request Entity Too Large", "", Print_None, ""},
{Event_MediaError, STRM_MSG_OPEN_ERROR, 414, 414, 104001, "Request URI Too Long", "", Print_None, ""},
{Event_MediaError, STRM_MSG_OPEN_ERROR, 452, 452, 104001, "Conference Not Found", "", Print_None, ""},
{Event_MediaError, STRM_MSG_OPEN_ERROR, 453, 453, 104001, "Not Enough Bandwidth", "", Print_None, ""},
{Event_MediaError, STRM_MSG_OPEN_ERROR, 456, 456, 104001, "Header Field Not Valid", "", Print_None, ""},
{Event_MediaError, STRM_MSG_OPEN_ERROR, 457, 457, 104001, "Invalid Range", "", Print_None, ""},
{Event_MediaError, STRM_MSG_OPEN_ERROR, 458, 458, 104001, "Parameter Is Read Only", "", Print_None, ""},
{Event_MediaError, STRM_MSG_OPEN_ERROR, 459, 459, 104001, "Aggregate Operation Not Allowed", "", Print_None, ""},
{Event_MediaError, STRM_MSG_OPEN_ERROR, 460, 460, 104001, "Only Aggregate Operation Allowed", "", Print_None, ""},
{Event_MediaError, STRM_MSG_OPEN_ERROR, 462, 462, 104001, "Destination Unreachable", "", Print_None, ""},
{Event_MediaError, STRM_MSG_OPEN_ERROR, 502, 502, 104001, "Bad Gateway", "", Print_None, ""},
{Event_MediaError, STRM_MSG_OPEN_ERROR, 504, 504, 104001, "Gateway Timeout", "", Print_None, ""},
{Event_MediaError, STRM_MSG_OPEN_ERROR, 600, 600, 104001, "The custom of an unknown error.", "", Print_None, ""},
{Event_MediaError, STRM_MSG_OPEN_ERROR, 601, 601, 104001, "Server bandwidth shortage", "", Print_None, ""},
{Event_MediaError, STRM_MSG_OPEN_ERROR, 602, 602, 104001, "The server is busy", "", Print_None, ""},
{Event_MediaError, STRM_MSG_OPEN_ERROR, 603, 603, 104001, "The access bandwidth is insufficient", "", Print_None, ""},
{Event_MediaError, STRM_MSG_OPEN_ERROR, 604, 604, 104001, "User does not have permissions", "", Print_None, ""},
{Event_MediaError, STRM_MSG_OPEN_ERROR, 605, 605, 104001, "Lack of user fees", "", Print_None, ""},
{Event_MediaError, STRM_MSG_RECV_IGMP_TIMEOUT, CODE_BASE, 10080, 104030, "Program broadcast temporarily abnormal.", "Error", Print_ErrUrl, "Description: After the STB joins a multicast group, no stream is delivered. IGMP URL:"},
{Event_MediaError, STRM_MSG_START_RECV_RTSP_TIMEOUT, CODE_BASE, 10082, 104015, "Program broadcast temporarily abnormal.", "Error", Print_ErrUrl, "Description: When playback starts, the interaction with the Media Server is normal but the STB does not receive media streams. Hms URL:"},
{Event_MediaError, STRM_MSG_PLAY_RECV_RTSP_TIMEOUT, CODE_BASE, 10086, 104015, "Program broadcast temporarily abnormal.", "Error", Print_ErrUrl, "Description: The STB is disconnected from the Media Server during playback, causing media stream interruption. Hms URL:"},  
{Event_MediaError, STRM_MSG_PACKETS_LOST, CODE_BASE, 10083, 0, "Network is busy, playing affected.", "Error", Print_ErrUrl, "Description: Severe packet loss occurs during streaming media playback, and the packet loss rate is greater than or equal to 1% in continuous two sampling periods (10s). Hms URL:"}, 
{Event_MediaError, STRM_MSG_INDEX_DAMAGE, CODE_BASE, 0, 104022, "", "Error", Print_None, ""},
{Event_MediaBuffer, HLS_MSG_BUFFER_BEGIN, CODE_BASE, 0, 104061, "", "", Print_None, ""},
{Event_MediaBuffer, HLS_MSG_BUFFER_END, CODE_BASE, 0, 104062, "", "", Print_None, ""}, 
{Event_MediaBuffer, HLS_MSG_BUFFER_LIMITED, CODE_BASE, 0, 104065, "", "", Print_None, ""}, 
{Event_MediaBuffer, HLS_MSG_BUFFER_DISKERROR, CODE_BASE, 0, 104064, "", "", Print_None, ""},
{Event_MediaBuffer, HLS_MSG_BUFFER_BANDWIDTH, CODE_BASE, 0, 104063, "", "", Print_None, ""},
{Event_MediaBegin, STRM_MSG_PTS_VIEW, CODE_BASE, 0, 0, "", "", Print_None, ""}, 
{Event_MediaBegin, STRM_MSG_SEEK_BEGIN, CODE_BASE, 0, 0, "", "", Print_None, ""}, 
{Event_MediaBegin, STRM_MSG_STREAM_BEGIN, CODE_BASE, 0, 0, "", "", Print_None, ""}, 
{Event_MediaEnd, STRM_MSG_SEEK_END, CODE_BASE, 0, 0, "", "", Print_None, ""}, 
{Event_MediaEnd, STRM_MSG_STREAM_END, CODE_BASE, 0, 0, "", "", Print_None, ""}, 
{Event_MediaADSBegin, STRM_MSG_ADVERTISE_BEGIN, CODE_BASE, 0, 0, "", "", Print_None, ""},
{Event_MediaADSEnd, STRM_MSG_ADVERTISE_END, CODE_BASE, 0, 0, "", "", Print_None, ""},
{Event_STBRestore, STRM_MSG_RECV_RTSP_RESUME, CODE_BASE, 0, 104015, "", "", Print_None, ""},
{Event_STBRestore, STRM_MSG_RECV_IGMP_RESUME, CODE_BASE, 0, 104030, "", "", Print_None, ""},   
};    
 
BrowserPlayerMsgCode::BrowserPlayerMsgCode()
{
    int tCount, i;

    tCount = sizeof(playerCodeArray) / sizeof(playerCodeArray[0]);
    for(i = 0; i< tCount; i++)
        addMsgCode(&playerCodeArray[i]);    
}

BrowserPlayerMsgCode::~BrowserPlayerMsgCode()
{
    
}

int
BrowserPlayerMsgCode::addMsgCode(PlayerCode *playCode)
{
    if (playCode == 0)
        return -1;
    mPlayerMsgCode.insert(std::make_pair(playCode->mStreamMsg + playCode->mStreamCode, playCode));
        
    return 0;    
        
}

PlayerCode*
BrowserPlayerMsgCode::getMsgCode(int message, int code)
{
    int keyValue = 0;
    std::map<int, PlayerCode *>::iterator it;
    
    if (message == STRM_MSG_OPEN_ERROR)
        keyValue = message + code;
    else
        keyValue = message + CODE_BASE;    
    
    it = mPlayerMsgCode.find(keyValue);

    if (it != mPlayerMsgCode.end())
        return it->second;
    else
        return 0;    
}    

static BrowserPlayerMsgCode *gBrowserPlayerMsgCode = NULL;

BrowserPlayerMsgCode &browserPlayerMsgCode()
{
    return *gBrowserPlayerMsgCode;
}

} // namespace Hippo
    

extern "C" void
BrowserPlayerCodeListCreate()
{
    Hippo::gBrowserPlayerMsgCode = new Hippo::BrowserPlayerMsgCode();
}
