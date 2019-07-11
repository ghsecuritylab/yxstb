
#include "JseHWDVBChannel.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"

/*
 *  获取 STB 上一次的频道系统自定义频道号(不分电视广播,所有频道在一起排列):
 *      var sValue = Utility.getValueByName(‘dvb_get_last_play_chanKey,{“type”:1}’)
 *  STB 上一次播放的视频频道系统频道号为 12 返回
 *      “12”
 *  type: 0:All channels 1:TV channels 2:Broadcast channels
 *  */
static int JseLastPlayByChankeyRead(const char* param, char* value, int len)
{
    return 0;
}

/*
 *  var sValue = Utility.getValueByName(‘dvb_get_chnls_count,{“type”:0}’)
 *      频道列表类型
 *          0 表示所有频道
 *          1 表示电视频道
 *          2 表示广播频道
 *  */
static int JseChannelCountRead(const char* param, char* value, int len)
{
    return 0;
}

/*
 *  从第一个频道开始获取 2 个频道信息:
 *     Var sJson =Utility.setValueByName(‘dvb_get_channel_list’,’{”type”:0,”position”:0,”count”:2 }’);
 *  STB 返回:
 *     {"channelNum":1,"channelList":[{”chanKey”:1,"tpID":1,"channelType":1,"channelName":"CCTV-1,"locked":0,"skip":0,"isHD":0,"Scrambled":0,”chanBandwidth“:3000,”HDCPEnable”:0,”CGMSAEnable”:0,”MacrovisionEnable”:0}]}
 *  */
static int JseChannelListRead(const char* param, char* value, int len)
{
	return 0;
}

/*
 *   获取 ID 为 2 的频道的参数信息
 *       Utility.getValueByName("dvb_get_channel_paras_by_chanKey,{"chanKey":2}");
 *   STB 返回:
 *      {”chanKey”:1,"tpID":1,"channelType":1,"channelName":"CCTV-1,"locked:0","skip":0,"isHD":0,"Scrambled": 0,”chanBandwidth“:3000,”HDCPEnable”:0,”CGMSAEnable”:0,”MacrovisionEnable”:0}
 *  */
static int JseChannelParasByChankeyRead(const char* param, char* value, int len)
{
    return 0;
}

static int JseChannelPositionByChankeyRead(const char* param, char* value, int len)
{
    return 0;
}

static int JseChannelNameWrite(const char* param, char* value, int len)
{
    return 0;
}


static int JseDeleteChannelWrite(const char* param, char* value, int len)
{
    return 0;
}


static int JseChnlSearchByNameRead(const char* param, char* value, int len)
{
    return 0;
}


static int JseDVBChannelDataCheckWrite(const char* param, char* value, int len)
{
    return 0;
}


static int JseChannelMoreDescriptionRead(const char* param, char* value, int len)
{
    return 0;
}

static int JseChannelRatingRead(const char* param, char* value, int len)
{
    return 0;
}


static int JseNewCchnlsCountRead(const char* param, char* value, int len)
{
    return 0;
}


static int JseNewChnlsInfoRead(const char* param, char* value, int len)
{
    return 0;
}

/*************************************************
Description: 初始化华为播放流控中的DVBS模块中的Channel定义的接口，由JseHWDVB.cpp调用
Input: 无
Return: 无
 *************************************************/
int JseHWDVBChannelInit()
{
    JseCall* call;

    //C20 regist
    call = new JseFunctionCall("dvb_get_last_play_chanKey", JseLastPlayByChankeyRead, 0);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dvb_get_chnls_count", JseChannelCountRead, 0);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dvb_get_channel_list", JseChannelListRead, 0);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dvb_get_channel_paras_by_chanKey", JseChannelParasByChankeyRead, 0);
    JseRootRegist(call->name(), call);

     //C20 regist
    call = new JseFunctionCall("dvb_get_channel_position_by_chanKey" , JseChannelPositionByChankeyRead, 0);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dvb_set_channel_name", 0, JseChannelNameWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dvb_delete_channel", 0, JseDeleteChannelWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dvb_chnl_search_by_name", JseChnlSearchByNameRead, 0);
    JseRootRegist(call->name(), call);

    //C20 regist,目前在JseHWPlay.cpp注册，这边实现的实现去那边删除
    call = new JseFunctionCall("dvbChannelDataCheck",  0, JseDVBChannelDataCheckWrite);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dvb_get_channel_more_description", JseChannelMoreDescriptionRead, 0);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dvb_get_channel_rating", JseChannelRatingRead, 0);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dvb_new_chnls_get_count", JseNewCchnlsCountRead, 0);
    JseRootRegist(call->name(), call);

    //C20 regist
    call = new JseFunctionCall("dvb_new_chnls_get_info", JseNewChnlsInfoRead, 0);
    JseRootRegist(call->name(), call);
    return 0;
}

