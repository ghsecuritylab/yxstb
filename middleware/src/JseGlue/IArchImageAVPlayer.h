#ifndef IARCHIMAGEAVPLAYER_H
#define IARCHIMAGEAVPLAYER_H

#include "JseGlueMacro.h"

class IArchImageAVPlayer {
public:
    virtual int ag_multicast_play() = 0;
    virtual int ag_multicast_stop() = 0;
    virtual int ag_multicast_getStatus() = 0;
    virtual int ag_multicast_getInfo() = 0;
    virtual int ag_multicast_block() = 0;
    virtual int ag_multicast_unblock() = 0;
    virtual int ag_audio_getMute() = 0;
    virtual int ag_audio_setMute() = 0;
    virtual int ag_audio_getVolume() = 0;
    virtual int ag_audio_setVolume() = 0;
    virtual int ag_audio_setStereo() = 0;
    virtual int ag_audio_getStereo() = 0;
    virtual int ag_audio_setTrack() = 0;
    virtual int ag_audio_getTrack() = 0;
    virtual int ag_audio_getNoOfTrack() = 0;
    virtual int ag_audio_setAudioPath() = 0;
    virtual int ag_audio_getAudioPath() = 0;
    virtual int ag_video_setTVSystem() = 0;
    virtual int ag_video_getTVSystem() = 0;
    virtual int ag_video_setWindow() = 0;
    virtual int ag_video_setAspectRatio() = 0;
    virtual int ag_video_getAspectRatio() = 0;
    virtual int ag_video_setDisplayMode() = 0;
    virtual int ag_video_getDisplayMode() = 0;
    virtual int ag_video_setAnalogueProtection() = 0;
    virtual int ag_video_getAnalogueProtection() = 0;
    virtual int ag_video_setMacrovision() = 0;
    virtual int ag_video_getMacrovision() = 0;
    virtual int ag_browser_setPosition() = 0;
    virtual int ag_browser_getPosition() = 0;
    virtual int ag_browser_setSize() = 0;
    virtual int ag_browser_getSize() = 0;
    virtual int ag_browser_hide() = 0;
    virtual int ag_browser_show() = 0;
    virtual int ag_browser_getStatus() = 0;
    virtual int ag_browser_setAlpha() = 0;
    virtual int ag_browser_getAlpha() = 0;
    virtual int ag_browser_refresh() = 0;
    virtual int ag_browser_setMemorySize() = 0;
    virtual int ag_browser_getMemorySize() = 0;
    virtual int ag_browser_setColorKey() = 0;
    virtual int ag_network_setLinkMode() = 0;
    virtual int ag_network_getLinkMode() = 0;
    virtual int ag_network_setIPAddress() = 0;
    virtual int ag_network_getIPAddress() = 0;
    virtual int ag_network_setSubnetMask() = 0;
    virtual int ag_network_getSubnetMask() = 0;
    virtual int ag_network_setDefaultGateway() = 0;
    virtual int ag_network_getDefaultGateway() = 0;
    virtual int ag_network_setDNSServer() = 0;
    virtual int ag_network_getDNSServer() = 0;
    virtual int ag_network_setNTPServer() = 0;
    virtual int ag_network_getNTPServer() = 0;
    virtual int ag_network_setPPPoEUserName() = 0;
    virtual int ag_network_getPPPoEUserName() = 0;
    virtual int ag_network_setPPPoEPasswd() = 0;
    virtual int ag_network_getPPPoEPasswd() = 0;
    virtual int ag_network_getMACAddress() = 0;
    virtual int ag_network_getNTPinfo() = 0;
    virtual int ag_network_saveNTPinfo() = 0;
    virtual int ag_time_setLocalTime() = 0;
    virtual int ag_time_getLocalTime() = 0;
    virtual int ag_time_setTimeZone() = 0;
    virtual int ag_time_getTimeZone() = 0;
    virtual int ag_time_sycToNTP() = 0;
    virtual int ag_vod_play() = 0;
    virtual int ag_vod_close() = 0;
    virtual int ag_vod_pause() = 0;
    virtual int ag_vod_resume() = 0;
    virtual int ag_vod_fast() = 0;
    virtual int ag_vod_seek() = 0;
    virtual int ag_vod_getStatus() = 0;
    virtual int ag_vod_setType() = 0;
    virtual int ag_vod_getType() = 0;
    virtual int ag_vod_setProtocol() = 0;
    virtual int ag_osd_create() = 0;
    virtual int ag_osd_delete() = 0;
    virtual int ag_osd_show() = 0;
    virtual int ag_osd_hide() = 0;
    virtual int ag_osd_getStatus() = 0;
    virtual int ag_osd_setAlpha() = 0;
    virtual int ag_osd_getAlpha() = 0;
    virtual int ag_osd_drawImage() = 0;
    virtual int ag_osd_showString() = 0;
    virtual int ag_osd_setFont() = 0;
    virtual int ag_osd_fillRect() = 0;
    virtual int ag_osd_clearRect() = 0;
    virtual int ag_dvb_setScanParameters() = 0;
    virtual int ag_dvb_startAutoScan() = 0;
    virtual int ag_dvb_abortAutoScan() = 0;
    virtual int ag_dvb_getAutoScanProgress() = 0;
    virtual int ag_dvb_manualScan() = 0;
    virtual int ag_dvb_play() = 0;
    virtual int ag_dvb_getCurrentPlayNumber() = 0;
    virtual int ag_dvb_stopPlay() = 0;
    virtual int ag_dvb_getTotalNumbers() = 0;
    virtual int ag_dvb_getLogicalChannelNumberByIndex() = 0;
    virtual int ag_dvb_getIndexByLogicalChannelNumber() = 0;
    virtual int ag_dvb_getChannelName() = 0;
    virtual int ag_dvb_deleteChannelList() = 0;
    virtual int ag_dvb_getCurrentFrequency() = 0;
    virtual int ag_dvb_getCurrentSignalQuality() = 0;
    virtual int ag_dvb_getSignalStrength() = 0;
    virtual int ag_dvb_startScan() = 0;
    virtual int ag_dvb_getLCNbyTriplet() = 0;
    virtual int ag_dvb_getParameter() = 0;
    virtual int ag_dvb_getNITData() = 0;
    virtual int ag_record_stop() = 0;
    virtual int ag_record_getStatus() = 0;
    virtual int ag_record_startDvbByLCN() = 0;
    virtual int ag_record_startIptvByIpAddr() = 0;
    virtual int ag_record_getDiskFreeSize() = 0;
    virtual int ag_record_getDiskTotalSize() = 0;
    virtual int ag_record_getNoOfDrives() = 0;
    virtual int ag_record_checkDiskStatus() = 0;
    virtual int ag_record_create() = 0;
    virtual int ag_record_startByID() = 0;
    virtual int ag_record_setMetaData() = 0;
    virtual int ag_record_getMetaData() = 0;
    virtual int ag_record_delMetaData() = 0;
    virtual int ag_record_getNoOfFile() = 0;
    virtual int ag_record_startByIPAndID() = 0;
    virtual int ag_record_startByLCNAndID() = 0;
    virtual int ag_playBack_rangeFile() = 0;
    virtual int ag_playBack_closeDatabase() = 0;
    virtual int ag_playBack_getFileInfo() = 0;
    virtual int ag_playBack_setFileName() = 0;
    virtual int ag_playBack_setFilePass() = 0;
    virtual int ag_playBack_Lock() = 0;
    virtual int ag_playBack_setFavorite() = 0;
    virtual int ag_playBack_deleteFile() = 0;
    virtual int ag_playBack_play() = 0;
    virtual int ag_playBack_stop() = 0;
    virtual int ag_playBack_pause() = 0;
    virtual int ag_playBack_resume() = 0;
    virtual int ag_playBack_trick() = 0;
    virtual int ag_playBack_getStatus() = 0;
    virtual int ag_playBack_getSpeed() = 0;
    virtual int ag_playBack_getCurrentTime() = 0;
    virtual int ag_playBack_getTotalTime() = 0;
    virtual int ag_playBack_getIndexByID() = 0;
    virtual int ag_playBack_seek() = 0;
    virtual int ag_system_getModeNo() = 0;
    virtual int ag_system_getSerialNum() = 0;
    virtual int ag_system_getHardwareVersion() = 0;
    virtual int ag_system_getSoftwareVersion() = 0;
    virtual int ag_system_runCmd() = 0;
    virtual int ag_frontPanel_showString() = 0;
    virtual int ag_frontPanel_turnOff() = 0;
    virtual int ag_frontPanel_lightControl() = 0;
    virtual int ag_power_standby() = 0;
    virtual int ag_power_wakeUp() = 0;
    virtual int ag_power_reboot() = 0;
    virtual int ag_homePage_set() = 0;
    virtual int ag_homePage_get() = 0;
    virtual int ag_upgrade_setMode() = 0;
    virtual int ag_upgrade_getMode() = 0;
    virtual int ag_upgrade_setURL() = 0;
    virtual int ag_upgrade_getURL() = 0;
    virtual int ag_upgrade_start() = 0;
    virtual int ag_upgrade_byURL() = 0;
    virtual int ag_upgrade_getUpgradePercent() = 0;
    virtual int ag_timeShift_pause() = 0;
    virtual int ag_timeShift_resume() = 0;
    virtual int ag_timeShift_stop() = 0;
    virtual int ag_timeShift_sync() = 0;
    virtual int ag_timeShift_trick() = 0;
    virtual int ag_timeShift_getspeed() = 0;
    virtual int ag_timeShift_getInfo() = 0;
    virtual int ag_timeShift_getStatus() = 0;
    virtual int ag_timeShift_getTotalTime() = 0;
    virtual int ag_timeShift_getRecordTim() = 0;
    virtual int ag_timeShift_getPlayTime() = 0;
    virtual int ag_timeShift_seek() = 0;
    virtual int ag_timeShift_getRecordTime() = 0;
    virtual int ag_timeShift_getBufferSize() = 0;
    virtual int ag_timeShift_setBufferSize() = 0;
    virtual int ag_download_open() = 0;
    virtual int ag_download_start() = 0;
    virtual int ag_download_pause() = 0;
    virtual int ag_download_resume() = 0;
    virtual int ag_download_getStatus() = 0;
    virtual int ag_download_delete() = 0;
    virtual int ag_download_getNumber() = 0;
    virtual int ag_download_getIdByIndex() = 0;
    virtual int ag_download_getInfoById() = 0;
    virtual int ag_download_play_start() = 0;
    virtual int ag_download_play_stop() = 0;
    virtual int ag_download_play_pause() = 0;
    virtual int ag_download_play_resume() = 0;
    virtual int ag_download_play_getStatus() = 0;
    virtual int ag_download_play_getPosition() = 0;
    virtual int ag_subtitle_show() = 0;
    virtual int ag_subtitle_hide() = 0;
    virtual int ag_subtitle_getTotalNumber() = 0;
    virtual int ag_subtitle_getLanguageByIndex() = 0;
    virtual int ag_subtitle_setLanguage() = 0;
    virtual int ag_teletext_setOutputMode() = 0;
    virtual int ag_teletext_open() = 0;
    virtual int ag_teletext_close() = 0;
    virtual int ag_teletext_show() = 0;
    virtual int ag_teletext_hide() = 0;
    virtual int ag_teletext_getTotalPageNum() = 0;
    virtual int ag_teletext_getFirstPage() = 0;
    virtual int ag_teletext_gotoPage() = 0;
    virtual int ag_teletext_pageUp() = 0;
    virtual int ag_teletext_pageDown() = 0;
    virtual int ag_teletext_subPageUp() = 0;
    virtual int ag_teletext_subPageDown() = 0;
    virtual int ag_settings_reset() = 0;
    virtual int printf() = 0;
    virtual int ag_dvb_init() = 0;
    virtual ~IArchImageAVPlayer() { }
};

extern IArchImageAVPlayer* createIArchImageAVPlayer(const char* name);

#endif //IARCHIMAGEAVPLAYER_H

