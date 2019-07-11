#ifndef __HIPPO_CONTEXT_API_H
#define __HIPPO_CONTEXT_API_H

#include <map>

#include <Hippo_api.h>
#include <hippo_module_config.h>
#include <Hippo_MediaPlayer.h>

namespace Hippo{

class HBrowserActiveObj;

/**
 *  JSE���溯��
 */
class HippoContext;

/***********************************************************/
/**
* @param   ctx: �������,�����е�HippoContextʵ�����
* @param   fieldName: �������,ioctlWrite/Read�е�Param;
* @param   fieldValue: �����������,
*                       ��ioctlWrite������ֵ,
*                       ��ioctlRead: �������Ĵ���Ҫ���,��֧��:�ŷָ�, fiedlNameΪ����, fieldValue�Ǹ�����Ĳ���.
* @param   aResult: �������; ����ʱ- 0Ϊִ��ioctlRead, ����ֵΪioctlWrite.
*							����Ϊ����ִ����ϸ���.
* @return  ��Ӧ��ִ�гɹ�/ʧ��, ���aResult���
* @remark ��
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
	 * HippoContext �����ӿ���, Ϊ������, ÿ�Խ�һ���µ�IPTVϵͳ
	 * �汾(����HuaweiC20,C10,C28�Ȱ汾)����Ҫһ����Ӧ��HippoContext,
	 * �ɸ����伯������,�Ӷ�Ӧ��Context������,��ά�����н��뱾ϵͳ��ص�.
	 * �ڻ���HippoContextά����ǰIPTVϵͳ��Ӧ��MediaPlayerMgr,���ڹ������MediaPlayerMgr.
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
  		 * ����HippoContextʵ��. ��������������ȡ����HippoSettings�����õİ汾��Ϣ.
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
        //HippoContext�������
        MediaPlayerMgr* m_pMediaPlayerMgr;
        HBrowserActiveObj* m_BrowserAObj;


		//IPTVƽ̨���.
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
        * ��ָ��Ϊƽ̨�汾, ��EPG�·�, ����������ô������֤�㷨;
        *	   01xx������
        *	   0101���Ϻ�����
        *	   0102���㶫����
        *	   0200����Ϊ
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

