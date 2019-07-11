#ifndef _SystemManager_H_
#define _SystemManager_H_

#include "LayerMixerDevice.h"
#include "LayerMixerDeviceZebra.h"
#include "Program/ProgramList.h"

#ifdef __cplusplus

#include <vector>

namespace Hippo {

class Resource;
class UltraPlayer;

class SystemManager {
public:
    SystemManager();
    ~SystemManager();

    LayerMixerDevice &mixer() { return mMixer; }

    ProgramList &channelList() { return mChannelList; }

    int attachMainPlayer(UltraPlayer *player);
    int detachMainPlayer(UltraPlayer *player);
    UltraPlayer *obtainMainPlayer();
    int releaseMainPlayer(UltraPlayer *player);
		
	int attachPipPlayer(UltraPlayer *player, int InstanceFlag/*0,pip; 1,mosaic*/);
	int detachPipPlayer(UltraPlayer *player, int InstanceFlag);
	UltraPlayer *obtainPipPlayer(int InstanceFlag);
	int releasePipPlayer(UltraPlayer *player, int InstanceFlag);
	void destoryPipPlayer(int InstanceFlag);
	int getMosaicPlayerNum(void);
	int setMosaicPlayerNum(int num,int flag);
	int getMosaicAllNum(void);
	int attachFreePlayer(UltraPlayer *player, int id);
	int detachFreePlayer(UltraPlayer *player);
	UltraPlayer *obtainMosaicPlayer(UltraPlayer *player);
	UltraPlayer *obtainFreePlayer(int id);
	int releaseFreePlayer(UltraPlayer *player);
	void destoryFreePlayer();

    void destoryMainPlayer();

    void destoryAllPlayer();

private:
#if defined(GRAPHIC_SINGLE_LAYER)
    LayerMixerDeviceZebra mMixer;
#else
    LayerMixerDevice mMixer;
#endif /*GRAPHIC_SINGLE_LAYER*/

    ProgramList mChannelList;

    UltraPlayer *mainPlayer;
    std::vector<UltraPlayer*> mPlayerArray;		
    std::vector<Resource*> mNetworkArray;
    std::vector<Resource*> mTunerArray;
    std::vector<Resource*> mDiskArray;
};

SystemManager &systemManager();

} // namespace Hippo

#endif // __cplusplus

#endif // _SystemManager_H_
