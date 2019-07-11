#ifndef _BrowserPlayerC20_H_
#define _BrowserPlayerC20_H_

#include "BrowserPlayer.h"
#ifdef __cplusplus

namespace Hippo {

class BrowserPlayerC20 : public BrowserPlayer {
public:
    BrowserPlayerC20(int id, player_type_e playerInstanceType);
    ~BrowserPlayerC20();

    virtual int joinChannel(int channelNumber);
    virtual int leaveChannel();
	virtual int play(int startTime, time_type_e timeType);
	virtual int stop();
	virtual int refreshVideoDisplay( );
	virtual int setProperty(player_property_type_e aType, HPlayerProperty& aValue);
	virtual int getProperty(player_property_type_e aType, HPlayerProperty& aResult);
	virtual int setSingleMedia(media_info_type_e, const char* mediaString);
	virtual const char * get(const char * ioStr);
private:
	int getSwitchChannelMode();
	std::string mChannelInfo;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _BrowserPlayerC20_H_
