#ifndef _MessageTypes_H_
#define _MessageTypes_H_

#ifdef __cplusplus

namespace Hippo {


} // namespace Hippo

#endif // __cplusplus

typedef enum {
	MessageType_Timer = 0,
	MessageType_System = 1,
	MessageType_KeyDown = 2,
	MessageType_KeyUp = 3,
	MessageType_Network = 4,
	MessageType_Char = 5,
	MessageType_Mouse = 6,
    MessageType_Android = 7,
	MessageType_Play = 14,
	MessageType_Repaint = 15,
	MessageType_Upgrade = 16,
	MessageType_Debug = 17,
	MessageType_LocalPlayer = 49,
	MessageType_Tr069 = 50,
	MessageType_PayShell = 51,
	MessageType_DLNA = 52,
	MessageType_JVM = 100,
	MessageType_ConfigSave = 0x200,
	MessageType_ClearVolumeIcon,
	MessageType_ClearMuteIcon,
	MessageType_ClearAudioTrackIcon,
	MessageType_ClearPlayStateIcon,
	MessageType_ShowChanLogoIcon,
	MessageType_ClearChanLogoIcon,
	MessageType_ShowDolbyIcon,
	MessageType_ClearDolbyIcon,
	MessageType_ClearAllIcon,
	MessageType_Prompt,
	MessageType_WaitClock,
    MessageType_SaveToFlash,
#if defined(Huawei_v5)
    MessageType_HDMI,
#endif
#if defined(Jiangsu)
    MessageType_ClearAuthLogo,
#endif
	MessageType_Report = 0x300,
	MessageType_NetworkDiagnose = 0x301,

#if defined(CYBERCLOUD)
	MessageType_CyberCloud = 0x350,
#endif
    MessageType_MaintenancePage, //message for local Maintenance page  
    MessageType_Unknow = 0x400
} MessageTypes;

#endif // _MessageTypes_H_
