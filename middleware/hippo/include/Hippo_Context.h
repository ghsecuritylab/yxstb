#ifndef __HIPPO_CONTEXT_API_H
#define __HIPPO_CONTEXT_API_H

#include <map>

#include <Hippo_api.h>
#include <hippo_module_config.h>
#include <Hippo_MediaPlayer.h>

namespace Hippo{

class HBrowserActiveObj;

/**
 *  JSE引擎函数
 */
class HippoContext;

/***********************************************************/
/**
* @param   ctx: 输入参数,运行中的HippoContext实例句柄
* @param   fieldName: 输入参数,ioctlWrite/Read中的Param;
* @param   fieldValue: 输入输出参数,
*                       对ioctlWrite是输入值,
*                       对ioctlRead: 如果输入的串需要拆分,仅支持:号分隔, fiedlName为命令, fieldValue是该命令的参数.
* @param   aResult: 输入参数; 输入时- 0为执行ioctlRead, 其他值为ioctlWrite.
*							输入为程序执行详细结果.
* @return  对应的执行成功/失败, 结果aResult输出
* @remark 无
* @see
* @author teddy      @date 2011/01/21
************************************************************/
//typedef bool (*ioctlFunc)( HippoContext* aCtx, HString& aFieldName, HString aFieldValue, int aResult );


	/**
	 * @file
	 * @author  Guohongqi <guohongqi@hybroad.com>
	 * @version 1.0
	 *
	 * @section LICENSE
	 *
	 * This program is free software; you can redistribute it and/or
	 * modify it under the terms of the GNU General Public License as
	 * published by the Free Software Foundation; either version 2 of
	 * the License, or (at your option) any later version.
	 *
	 * This program is distributed in the hope that it will be useful, but
	 * WITHOUT ANY WARRANTY; without even the implied warranty of
	 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
	 * General Public License for more details at
	 * http://www.gnu.org/copyleft/gpl.html
	 *
	 * @section DESCRIPTION
	 *
	 * HippoContext 基础接口类, 为纯虚类, 每对接一个新的IPTV系统
	 * 版本(例如HuaweiC20,C10,C28等版本)都需要一个对应的HippoContext,
	 * 可根据其集成特性,从对应的Context类派生,其维护所有仅与本系统相关的.
	 * 在基类HippoContext维护当前IPTV系统对应的MediaPlayerMgr,用于管理各个MediaPlayerMgr.
	 *
	 */
    template<typename T >
    struct ioctlMapNode{
        T m_ioctlRead;
        T m_ioctlWrite;
        ioctlMapNode<T>& operator= (ioctlMapNode<T> me )
        {
            m_ioctlRead = me.m_ioctlRead;
            m_ioctlWrite = me.m_ioctlWrite;
            return *this;
        }

        ioctlMapNode( ) : m_ioctlRead(0), m_ioctlWrite(0) { };
        ioctlMapNode( T& t1, T& t2 ): m_ioctlRead( t1 ), m_ioctlWrite( t2 ) {}
    };
    typedef std::map<HString, ioctlMapNode<JseIoctlFunc> > ioctlCustomMap;
	class HippoContext {
        typedef bool (HippoContext::*ioctlFunc)(HString& aFieldName, HString& aFieldParam, HString& aFieldValue, int aResult );
        typedef std::map<const HString, ioctlMapNode<ioctlFunc> > ioctlMap;
    public:
        typedef HippoContext* (*CreateHippoContext)( void* );
        typedef std::map<const HString, CreateHippoContext> HippoContextFactoryMap;
    public:
        static HString s_NullString;
		static const int s_ioctlWriteFlag;
        //middleware type.
    private:
        static HippoContextFactoryMap s_createMap; //MediaPlayerMgr type and createFactory map.
        static bool s_bInited;
        static HippoContext* s_pInstance;

    public:
		/**
  		 * 创建HippoContext实例. 具体事例化的类取决于HippoSettings中配置的版本信息.
  		 */
        static void Init( const char* aUrl );
        static HippoContext* getContextInstance( const char* aType = NULL );
        static int registerHippoContextType( const char* , CreateHippoContext );
        static int unregisterHippoContextType( const char* );

        virtual ~HippoContext( );
        virtual void Init( );
        virtual int JseRegister( const char* ioName, JseIoctlFunc& rfunc, JseIoctlFunc& wfunc, ioctl_context_type_e eChnl );
        virtual int UnJseRegister( const char* ioName, ioctl_context_type_e eChnl );

        MediaPlayerMgr* getMediaPlayerMgr( const char* aType = MEDIAPLAYERMGR_TYPE_CTC );
        HBrowserActiveObj* getBrowserActiveObj( ){ return m_BrowserAObj; }

        //interface corresponding to CTC IPTV2.0 /HW 2.0 Authentication::CTCSetConfig
        virtual int AuthenticationCTCSetConfig( HString& aField, HString& aValue );
        virtual int AuthenticationCTCGetConfig( HString& aField, HString& aValue );
        virtual int AuthenticationCTCGetAuthInfo( HString& aToken, HString& aResult );
        virtual int AuthenticationCUSetConfig( HString& aField, HString& aValue ) = 0;
		virtual int AuthenticationCUGetConfig( HString& aField, HString& aValue ) = 0;
		virtual int AuthenticationCUGetAuthInfo( HString& aToken, HString& aResult ) = 0;

        //intreface corresponding to iPanel.ioctlWrite/ioctlRead.
        virtual int ioctlWrite( HString& aField, HString& aValue /*in*/ );
        virtual int ioctlRead( HString& aField, HString& aValue /*out*/ );

        //return global info.
        virtual HString& ServiceEntry( int aKey );

        HString& UserToken( ){ return m_UserToken; }
        HString& EPGDomainBackup( ){ return m_EPGDomainBackup; }
        HString& EPGDomain( );
        HString& EPGGroupNMB( ){ return m_EPGGroupNMB; }
        int UserGroupNMB( ){ return m_UserGroupNMB; }
        HString& NTPDomainBackup( ){ return m_NTPDomainBackup; }
        HString& NTPDomain( ){ return m_NTPDomain; }
        HString& ManagementDomainBackup( ){ return m_ManagementDomainBackup; }
        HString& ManagementDomain( ){ return m_ManagementDomain; }
        HString& UpgradeDomainBackup( ){ return m_UpgradeDomainBackup; }
        HString& UpgradeDomain( ){ return m_UpgradeDomain; }

    protected:
        int UserToken( HString& aStr ) { m_UserToken = aStr; return 0; }
        int EPGDomain( const char* aStr );
        int EPGGroupNMB( HString& aStr );
        int UserGroupNMB( HString& aStr );
        int NTPDomainBackup( HString& aStr );
        int NTPDomain( HString& aStr );
        int EPGDomainBackup( HString& aStr );
        int ManagementDomainBackup( HString& aStr );
        int ManagementDomain( HString& aStr );
        int UpgradeDomainBackup( HString& aStr );
        int UpgradeDomain( HString& aStr );
    protected:
        HippoContext( );

    protected:
        //HippoContext自身变量
        MediaPlayerMgr* m_pMediaPlayerMgr;
        HBrowserActiveObj* m_BrowserAObj;


		//IPTV平台相关.
        HString m_UserToken;
        HString m_EPGGroupNMB;

        HString m_NTPDomainBackup;
        HString m_NTPDomain;
        HString m_ManagementDomainBackup;
        HString m_ManagementDomain;
        HString m_UpgradeDomainBackup;
        HString m_UpgradeDomain;
        HString m_EPGDomainBackup;
        HString m_EPGDomain;
		int m_UserGroupNMB;

        /*
        * 特指华为平台版本, 由EPG下发, 决定采用怎么样的认证算法;
        *	   01xx：电信
        *	   0101：上海电信
        *	   0102：广东电信
        *	   0200：华为
        */
        HString m_PlatformCode;
        int m_UserStatus;
    private:
        ioctlMap m_ioctlMap;
        ioctlCustomMap m_ioctlCustomMap;
	};
}

#ifdef __cplusplus
extern "C" {
#endif

	int a_HippoContextCTC_Init( );
	int a_HippoContextHW_Init( );
    int a_HippoContextHWC10_Init( );
    int a_HippoContextHWC20_Init( );

#ifdef __cplusplus
}
#endif

#endif

