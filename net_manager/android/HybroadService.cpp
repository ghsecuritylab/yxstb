
#include <binder/IServiceManager.h>
#include <utils/Log.h>
#include "HybroadService.h"
#include <stdlib.h>
#include "param_map.h"
#include "tr069_interface.h"
#include "tr069_log.h"
#include "nm_dbg.h"
#include "tr069_port_PacketCapture.h"
void sendMsgToMonitor(char *name, char *str, unsigned int str_len);

#define MGMT_MT_UDISK_OUT            0x1111
#define MGMT_HYBROAD_TCPDUMP_DOWN    0x1112

#define MGMT_HYBROAD_UPGRADE_EXIT    0x1501

namespace android {

	nm_client_node::nm_client_node()
	{
		pthread_mutex_init(&node_mux, NULL);
		nm_client_param = NULL;

	}

	nm_client_node:: ~nm_client_node()
	{
	}


	int HybroadService::connect_flag = 0;

	void* nm_client_process(void *param);

	HybroadService::HybroadService()
	{
		nm_msg_level(LOG_DEBUG, "HybroadService()");
		pthread_mutex_init(&nm_client_mux, NULL);
		pthread_mutex_init(&client_vector_mux, NULL);
		pthread_create(&nm_client_pid, NULL, nm_client_process, this);
		nm_msg_level(LOG_DEBUG, "HybroadService()");
		
	}


	HybroadService::~HybroadService() 
	{
		nm_msg_level(LOG_DEBUG, "~HybroadService()");
	}

	void HybroadService::instantiate() 
	{
		nm_msg_level(LOG_DEBUG,"START HybroadService::instantiate()");
		nm_msg_level(LOG_DEBUG,"Process:%d instantiate", getpid());
		sp<IServiceManager> sm = defaultServiceManager();
		sm->addService(String16("com.hybroad.stb"), new HybroadService());
		nm_msg_level(LOG_DEBUG,"END HybroadService::instantiate()");
	}

	int HybroadService::arpping(const char *str) 
	{
		nm_msg_level(LOG_DEBUG,"START HybroadService::arpping()");
		nm_msg_level(LOG_DEBUG,"Process:%d,HybroadServer::arpping %s",getpid(), str);
		//mListener->PostAnswer(10);
		nm_msg_level(LOG_DEBUG,"END HybroadService::arpping");
		return 0;
	}

	int HybroadService::ping(const char *str) 
	{
		nm_msg_level(LOG_DEBUG,"START HybroadService::ping");
		nm_msg_level(LOG_DEBUG,"Process:%d HybroadServer::ping %s", getpid(),str);
		//mListener->PostAnswer(10);
		nm_msg_level(LOG_DEBUG,"END HybroadService::ping");
		return 0;
	}

	char * HybroadService::nm_get_value(char *name)
	{
		char *value_buf = (char *)malloc(8 * 1024);
		
		nm_msg_level(LOG_DEBUG,"START HybroadService::nm_get_value\n");
		tr069_api_getValue(name, value_buf,  8 * 1024);
		return value_buf;

	}

	void HybroadService::nm_set_value(char *name, char *str, unsigned int str_len) 
	{
		nm_msg_level(LOG_DEBUG,"START HybroadService::nm_set_value");

		tr069_api_setValue(name, str, str_len);
	}
	
	void HybroadService::sendToMonitor(char *name, char *str, unsigned int str_len) 
	{
		nm_msg_level(LOG_DEBUG,"START HybroadService::sendMsgToMonitor");

		sendMsgToMonitor(name, str, str_len);
	}

	int HybroadService::setListener(const sp<ICallbackListener> &listener)
	{
		 nm_msg_level(LOG_DEBUG,"START HybroadService::setListener");
		 nm_client_node *node = new nm_client_node();
		 node->client_listener = listener;
		 pthread_mutex_lock(&client_vector_mux);
		 client_vector.push_back(node);
		 pthread_mutex_unlock(&client_vector_mux);
		 pthread_mutex_unlock(&nm_client_mux);
		 mListener = listener;
		 connect_flag = 1;
		 nm_msg_level(LOG_DEBUG,"END HybroadService::setListener");

		 return 0;
	}

	status_t HybroadService::onTransact(uint32_t code, const Parcel &data, Parcel *reply, uint32_t flags) 
	{
		const char *str;
		//nm_msg("START HybroadService::onTransact");
		nm_msg_level(LOG_DEBUG,"CODE = %d..\n", code);
		switch (code) {
			case TRANSACTION_arpping:
				CHECK_INTERFACE(IHybroadService, data, reply);
				str = data.readCString();
				reply->writeInt32(arpping(str));
				return NO_ERROR;
				break;
			case TRANSACTION_ping:
				CHECK_INTERFACE(IHybroadService, data, reply);
				str = data.readCString();
				reply->writeInt32(ping(str));
				return NO_ERROR;
				break;
			case TRANSACTION_setListener:
				CHECK_INTERFACE(IHybroadService, data, reply);
				setListener(interface_cast<ICallbackListener>(data.readStrongBinder()));
				reply->writeInt32(0);
				break;
			case TRANSACTION_nm_getvalue:{
				CHECK_INTERFACE(IHybroadService, data, reply);
				nm_msg_level(LOG_DEBUG,"TRANSACTION_GetValue start\n");
				
				String16 str1_read = data.readString16();
				char c_get_read[1024] = {0};
				utf16_to_utf8(str1_read.string(), str1_read.size(), c_get_read);
				nm_msg_level(LOG_DEBUG,"getvalue get ...%s \n", c_get_read);
				
				char* result_read = nm_get_value(c_get_read);
				nm_msg_level(LOG_DEBUG,"getvalue ret....%s \n", result_read);
				if(result_read != NULL){
					String16 result_read16(result_read);
					int getValueLength = result_read16.size();
					
					reply->writeNoException();
					//send length
					//reply->writeInt32(getValueLength);
					//send value
				        reply->writeString16(result_read16);
					free(result_read);
				} else {
					nm_msg("fail to get value of %s\n", c_get_read);
				}
				return NO_ERROR;
				break;
			}
				
			case TRANSACTION_nm_setvalue:{
				CHECK_INTERFACE(IHybroadService, data, reply);
				char c_set_value1[1024] = {0};
				char c_set_value2[1024] = {0};

				String16 str1_value = data.readString16();
				utf16_to_utf8(str1_value.string(), str1_value.size(), c_set_value1);
				nm_msg_level(LOG_DEBUG,"SetValue.....%s\n", c_set_value1);

				String16 str2_value = data.readString16();
				utf16_to_utf8(str2_value.string(), str2_value.size(), c_set_value2);
				nm_msg_level(LOG_DEBUG,"SetValue.....%s\n", c_set_value2);
				int len_buf = (data.readInt32());
				nm_set_value(c_set_value1, c_set_value2, len_buf);				
				break;
			}
			
			case TRANSACTION_sendToMonitor:{
				nm_msg_level(LOG_DEBUG,"TRANSACTION_sendToMonitor..\n");
				CHECK_INTERFACE(IHybroadService, data, reply);
				char c_set_value1[1024] = {0};
				char c_set_value2[1024] = {0};

				String16 str1_value = data.readString16();
				utf16_to_utf8(str1_value.string(), str1_value.size(), c_set_value1);
				nm_msg_level(LOG_DEBUG,"sendToMonitor.....%s\n", c_set_value1);

				String16 str2_value = data.readString16();
				utf16_to_utf8(str2_value.string(), str2_value.size(), c_set_value2);
				nm_msg_level(LOG_DEBUG,"sendToMonitor.....%s\n", c_set_value2);
				int len_buf = (data.readInt32());
				sendToMonitor(c_set_value1, c_set_value2, len_buf);				
				break;
			}
			
			default:
				return BBinder::onTransact(code, data, reply, flags);
		    }
			//nm_msg_level(LOG_DEBUG, "END HybroadService::onTransact()");
	    return 0;
	}

	int HybroadService::Read(HMW_MgmtConfig_SRC eSrcType, const char* szParm, char* pBuf, int iLen)
	{
		nm_msg_level(LOG_DEBUG,"HybroadService::Read %s\n", szParm);
		sp<ICallbackListener> temp_listener;

		temp_listener = listener_handle_get_byparam(szParm);
		if(temp_listener != NULL){
			nm_msg_level(LOG_DEBUG,"temp_listener::Read %s \n", szParm);
			char *tmpBuf = temp_listener->ReadCallback(eSrcType, szParm, iLen);
			strcpy(pBuf, tmpBuf);
			free(tmpBuf);
		}
		nm_msg_level(LOG_DEBUG,"HybroadService::Read end\n");

		return 0;
	}


	int HybroadService::Write(HMW_MgmtConfig_SRC eSrcType, const char* szParm, char* pBuf, int iLen)
	{
		nm_msg_level(LOG_DEBUG,"HybroadService::Write %s\n", szParm);
		sp<ICallbackListener> temp_listener;

		temp_listener = listener_handle_get_byparam(szParm);
		if(temp_listener != NULL){
			temp_listener->WriteCallback(eSrcType, szParm, pBuf, iLen);
		}
		return 0;
	}

	int HybroadService::Notify(HMW_MgmtMsgType eMsgType, unsigned int argc, void * argv[])
	{
		char *strRet = NULL;
		char parm1[100] = {0};
		char parm2[100] = {0};
		sp<ICallbackListener> temp_listener_setting;
		sp<ICallbackListener> temp_listener_iptv;

		temp_listener_setting = listener_handle_get_bymodulename("setting");
		temp_listener_iptv = listener_handle_get_bymodulename("IPTV");
			
		nm_msg_level(LOG_DEBUG, "HybroadService::Notify\n");

		switch (eMsgType) {
			case MGMT_MT_PLAYER_BY_CHANNO:
				sprintf(parm1, "%d", (*(int *)argv[0]));
				//printf("test:MGMT_MT_PLAYER_BY_CHANNO,[%d]\n", (*(int *)argv[0]));
				if (temp_listener_iptv != NULL) {
					strRet = temp_listener_iptv->NotifyCallback(eMsgType, parm1, "");
					*((int *)argv[1]) = atoi(strRet);
				}

				break;
				
			case MGMT_MT_PLAYER_BY_URL:
				//printf("test:MGMT_MT_PLAYER_BY_URL[%s]\n", (const char *)argv[0]);
				if (temp_listener_iptv != NULL) {
				    strRet = temp_listener_iptv->NotifyCallback(eMsgType, (const char *)argv[0], "");
				    *((int *)argv[1]) = atoi(strRet);
				}
				break;
				
			case MGMT_MT_PLAYER_STOP:
				if (temp_listener_iptv != NULL) {
					strRet = temp_listener_iptv->NotifyCallback(eMsgType, "", "");
					*((int *)argv[0]) = atoi(strRet);
					printf("%d...\n", *((int *)argv[0]));
				}
				break;
				
			case MGMT_MT_PLAYER_MPCTRL:
				if (temp_listener_iptv != NULL) {
					strRet = temp_listener_iptv->NotifyCallback(eMsgType, (const char *)argv[0], "");
				}
				break;

			case MGMT_CPE_CONFIG_FACTORYRESET:
				if (temp_listener_setting != NULL) {
					strRet = temp_listener_setting->NotifyCallback(eMsgType, "", "");
				}
				break;
				
			case MGMT_MT_TOOL_REBOOT:
				if (temp_listener_setting != NULL) {
					strRet = temp_listener_setting->NotifyCallback(eMsgType, "", "");
				}
				break;
				
			case MGMT_MT_ENTER_DEBUG:
				if (temp_listener_setting != NULL) {
					strRet = temp_listener_setting->NotifyCallback(eMsgType, "", "");
				}
				break;
				
			case MGMT_MT_EXIT_DEBUG:
				if (temp_listener_setting != NULL) {
					strRet = temp_listener_setting->NotifyCallback(eMsgType, "", "");
				}
				break;
				
			case MGMT_MT_GET_CHANNELNUM_TOTAL:
				if (temp_listener_iptv != NULL) {
					strRet = temp_listener_iptv->NotifyCallback(eMsgType, "", "");
					*((int *)argv[0]) = atoi(strRet);
				}
				break;
				
			case MGMT_MT_GET_CHANNELINFO_I:
				if (temp_listener_iptv != NULL) {
					sprintf(parm1, "%d", (*(int *)argv[0]));
					strRet = temp_listener_iptv->NotifyCallback(eMsgType, parm1, "");
					strcpy((char *)argv[1], strRet);
				}
				break;
				
			case MGMT_MT_GET_COLLECT_FILEPATH:
				break;
				
			case MGMT_MT_UPGRADE_SET_DOWNLOAD_PER:
				if (temp_listener_setting != NULL) {
					//printf("test:JNI, argv0[%lld],argv1[%lld]\n", *(long long *)argv[0], *(long long *)argv[1]);
					sprintf(parm1, "%lld", (*(long long *)argv[0]));
					sprintf(parm2, "%lld", (*(long long *)argv[1]));
					strRet = temp_listener_setting->NotifyCallback(eMsgType, parm1, parm2);
				}
				break;

			case MGMT_MT_UPGRADE_SET_BURN_START:
				if (temp_listener_setting != NULL) {
					strRet = temp_listener_setting->NotifyCallback(eMsgType, "", "");
				}
				break;
			
			case MGMT_MT_UDISK_OUT:
				if (temp_listener_iptv != NULL) {
					strRet = temp_listener_setting->NotifyCallback(eMsgType, "", "");
				}
				break;
			case MGMT_HYBROAD_TCPDUMP_DOWN:
				nm_msg_level(LOG_DEBUG, "HybroadService::Notify,MGMT_HYBROAD_TCPDUMP_DOWN\n");
				if (temp_listener_iptv != NULL) {
					strRet = temp_listener_iptv->NotifyCallback(eMsgType, "", "");
				}
				break;
      case MGMT_HYBROAD_UPGRADE_EXIT:
				nm_msg_level(LOG_DEBUG, "HybroadService::Notify,MGMT_HYBROAD_UPGRADE_EXIT\n");
				if (temp_listener_setting != NULL) {
					strRet = temp_listener_setting->NotifyCallback(eMsgType, "", "");
				}
				break;
				
			default:
				break;
			
			}
			if (strRet != NULL)
				free(strRet);
		return 0;
	}

	int HybroadService::LogExp(const char* pszFile, int lLine, const char* pszFunc, int lThreadId, int enLogType, int enLogLevel, const char* pszModule, const char* format, ...)
	{
		if (mListener != NULL)
			mListener->LogExpCallback(pszFile, lLine, pszFunc, lThreadId, enLogType, enLogLevel, pszModule, format);
		return 0;
	}

	//tr069
	const  char *log_param = "Device.X_00E0FC.LogParaConfiguration";
	const  char *packetCapture = "Device.X_00E0FC.PacketCapture.";
	int HybroadService::GetValue(char *name, char *str, int str_len)
	{
		//nm_msg_level(LOG_DEBUG,"HybroadService::GetValue:%s\n", name);
		sp<ICallbackListener> temp_listener;
		local_get_value get = NULL;

		if(str == NULL){
			nm_msg("error,str is NULL, skip\n");
			return -1;
		}
		//check local service
		get = nm_local_param_check_read(name);
		if(get)
			return get(name, str, str_len);
		//end of check local service
		temp_listener = listener_handle_get_byparam(name);
		if(temp_listener != NULL){

			//nm_msg_level(LOG_DEBUG,"param is match, read it\n");
			char *tempBuf = temp_listener->GetValue(name);
			//nm_msg_level(LOG_DEBUG,"HybroadService::GetValue:%s, buf len is %d\n", tempBuf, str_len);
			strncpy(str, tempBuf, str_len);
			free(tempBuf);
			return 0;
		}else if(strncmp(name, log_param, strlen(log_param)) == 0){//日志上报系统
			char temp_buf[128 + 1];
			tr069_log_param_get(name + strlen(log_param) + 1, temp_buf, 128);
			strncpy(str, temp_buf, str_len);
			return 0;
		}else if(strncmp(name, packetCapture, strlen(packetCapture)) == 0){
			tr069GetPacketCaptureParamValue(name + strlen(packetCapture), str, 128);
			return 0;
		} else {
			nm_msg("unknown param:%s, ignored it\n", name);
			return -1;
		}
		return -1;
	}

	int HybroadService::SetValue(char *name, char *str,  int pval)
	{

		//nm_msg_level(LOG_DEBUG,"HybroadService::SetValue\n");
		sp<ICallbackListener> temp_listener;
		local_set_value set = NULL;

		//check local service
		set = nm_local_param_check_write(name);
		if(set)
			return set(name, str);
		//end of check local service
           
		temp_listener = listener_handle_get_byparam(name);
		if(temp_listener != NULL){
			temp_listener->SetValue(name, str, pval);
			return 0;
		} else if(strncmp(name, log_param, strlen(log_param)) == 0){//日志上报系统
			tr069_log_param_set(name + strlen(log_param) + 1, str);
			return 0;
		}else if(strncmp(name, packetCapture, strlen(packetCapture)) == 0){
			tr069SetPacketCaptureParamValue(name + strlen(packetCapture) ,str, pval);
			return 0;		   
	       }
		return -1;
      }
	
	void* HybroadService::nm_client_process(void *param)
	{
		HybroadService *p_service;
		nm_client_vector_t::iterator it;
		char *cache_buf;

		p_service = (HybroadService *)param;
		nm_track();
		//sleep(5);
		//wait for client comming.
		while(1){
		nm_track();
			pthread_mutex_lock(&p_service->nm_client_mux);
			usleep(50 * 1000);
			nm_msg_level(LOG_DEBUG,"loop\n");
			
			pthread_mutex_lock(&p_service->client_vector_mux);

			if (p_service->client_vector.size() == 0) {
			    pthread_mutex_unlock(&p_service->client_vector_mux);
			    continue;
			}

            
            //从后向前遍例，避免找到的是以前注册的相同模块名的listener
			it = p_service->client_vector.end();
			it--;
			for (;;it--) {
			    if (!(*it)->nm_client_param){
					nm_msg_level(LOG_DEBUG,"get module param\n");
					char *temp_buf = (*it)->client_listener->GetCapability();
					if(temp_buf){
						cache_buf = strdup(temp_buf);
						(*it)->nm_client_param = module_param_get(cache_buf);
						free(cache_buf);
						nm_msg_level(LOG_DEBUG, "success to get module param\n");
					} else {
						nm_msg("fail to get module param\n");
					}
			    }
				if (it == p_service->client_vector.begin())
					break;
			}
			
			pthread_mutex_unlock(&p_service->client_vector_mux);			
		}
		return NULL;
	}

	sp<ICallbackListener> HybroadService::listener_handle_get_byparam(const char *param_name)
	{
		//nm_msg_level(LOG_DEBUG, "HybroadService::Read\n");
		nm_client_vector_t::iterator it;
		sp<ICallbackListener> temp_listener = NULL;

		pthread_mutex_lock(&client_vector_mux);

		if (client_vector.size() == 0) {
		    pthread_mutex_unlock(&client_vector_mux);
		    return temp_listener;
		}
		it = client_vector.end();
		it--;
		for (;;it--) {
			//从后向前遍例，避免找到的是以前注册的相同模块名的listener
		    if ((*it)->nm_client_param){
				if(param_check((*it)->nm_client_param, (char *)param_name) == 1){
					pthread_mutex_unlock(&client_vector_mux);
					return (*it)->client_listener;
				}
		    }
			if (it == client_vector.begin())
					break;
		}
		
		pthread_mutex_unlock(&client_vector_mux);
		return temp_listener;
	}

	sp<ICallbackListener> HybroadService::listener_handle_get_bymodulename(const char *module_name)
	{
		//nm_msg_level(LOG_DEBUG, "HybroadService::Read\n");
		nm_client_vector_t::iterator it;

		pthread_mutex_lock(&client_vector_mux);

        //从后向前遍例，避免找到的是以前注册的相同模块名的listener
        if (client_vector.size() == 0) {
            pthread_mutex_unlock(&client_vector_mux);
            return NULL;
        }
        it = client_vector.end();
		it--;
		for (;;it--) {
		    if ((*it)->nm_client_param){
				if(module_name_check((*it)->nm_client_param, (char *)module_name) == 1){
					pthread_mutex_unlock(&client_vector_mux);
					return (*it)->client_listener;
				}
		    }
			if (it == client_vector.begin())
					break;
		}
		
		pthread_mutex_unlock(&client_vector_mux);
		return NULL;
	}

	int HybroadService::module_is_registered(const char *module_name)
	{
		if(listener_handle_get_bymodulename(module_name) != NULL)
			return 1;
		return 0;
	}
}
