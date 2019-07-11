
#include "JseHWDVBChannel.h"
#include "JseRoot.h"
#include "JseFunctionCall.h"

/*
 *  ��ȡ STB ��һ�ε�Ƶ��ϵͳ�Զ���Ƶ����(���ֵ��ӹ㲥,����Ƶ����һ������):
 *      var sValue = Utility.getValueByName(��dvb_get_last_play_chanKey,{��type��:1}��)
 *  STB ��һ�β��ŵ���ƵƵ��ϵͳƵ����Ϊ 12 ����
 *      ��12��
 *  type: 0:All channels 1:TV channels 2:Broadcast channels
 *  */
static int JseLastPlayByChankeyRead(const char* param, char* value, int len)
{
    return 0;
}

/*
 *  var sValue = Utility.getValueByName(��dvb_get_chnls_count,{��type��:0}��)
 *      Ƶ���б�����
 *          0 ��ʾ����Ƶ��
 *          1 ��ʾ����Ƶ��
 *          2 ��ʾ�㲥Ƶ��
 *  */
static int JseChannelCountRead(const char* param, char* value, int len)
{
    return 0;
}

/*
 *  �ӵ�һ��Ƶ����ʼ��ȡ 2 ��Ƶ����Ϣ:
 *     Var sJson =Utility.setValueByName(��dvb_get_channel_list��,��{��type��:0,��position��:0,��count��:2 }��);
 *  STB ����:
 *     {"channelNum":1,"channelList":[{��chanKey��:1,"tpID":1,"channelType":1,"channelName":"CCTV-1,"locked":0,"skip":0,"isHD":0,"Scrambled":0,��chanBandwidth��:3000,��HDCPEnable��:0,��CGMSAEnable��:0,��MacrovisionEnable��:0}]}
 *  */
static int JseChannelListRead(const char* param, char* value, int len)
{
	return 0;
}

/*
 *   ��ȡ ID Ϊ 2 ��Ƶ���Ĳ�����Ϣ
 *       Utility.getValueByName("dvb_get_channel_paras_by_chanKey,{"chanKey":2}");
 *   STB ����:
 *      {��chanKey��:1,"tpID":1,"channelType":1,"channelName":"CCTV-1,"locked:0","skip":0,"isHD":0,"Scrambled": 0,��chanBandwidth��:3000,��HDCPEnable��:0,��CGMSAEnable��:0,��MacrovisionEnable��:0}
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
Description: ��ʼ����Ϊ���������е�DVBSģ���е�Channel����Ľӿڣ���JseHWDVB.cpp����
Input: ��
Return: ��
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

    //C20 regist,Ŀǰ��JseHWPlay.cppע�ᣬ���ʵ�ֵ�ʵ��ȥ�Ǳ�ɾ��
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

