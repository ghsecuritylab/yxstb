
#include "VersionSetting.h"

#include "Assertions.h"
#include "stbinfo/stbinfo.h"
#include "build_info.h"
#include "ind_cfg.h"
#include "ind_mem.h"
#include "mid/mid_mutex.h"
#include "SettingApi.h"
#include "sys_basic_macro.h"
#include "SettingEnum.h"

#include <stdio.h>
#include <string.h>

typedef struct sys_upgrade_version
{
    int dirty;
    int softwareVersion;
    int logoVersion;
    int settingVersion;
    int templateVersion;
    int failureNum;
    char CFversion[USER_LEN];
    int softwareVersionBak;

} SysSoftwareVersionInfo;

static SysSoftwareVersionInfo m_versionInfo;
static CfgTree_t g_cfgTree = NULL;

void versionSettingInit(void)
{
	if(g_cfgTree)
        return ;

    g_cfgTree = ind_cfg_create();
    if(g_cfgTree == NULL) {
        LogSafeOperError("tree_cfg_create\n");
        return ;
    }

    ind_cfg_inset_object(g_cfgTree, "version");
    ind_cfg_inset_int(g_cfgTree, "version.normal", &m_versionInfo.softwareVersion);
    ind_cfg_inset_int(g_cfgTree, "version.logo", &m_versionInfo.logoVersion);
    ind_cfg_inset_int(g_cfgTree, "version.config", &m_versionInfo.settingVersion);
    ind_cfg_inset_int(g_cfgTree, "version.templateVersion", &m_versionInfo.templateVersion);
    ind_cfg_inset_int(g_cfgTree, "version.failure", &m_versionInfo.failureNum);
    ind_cfg_inset_string(g_cfgTree, "version.CFversion", m_versionInfo.CFversion, USER_LEN);
#ifdef INCLUDE_LITTLESYSTEM
    ind_cfg_inset_int(g_cfgTree, "version.normalBak", &m_versionInfo.softwareVersionBak);
#endif
    memset(&m_versionInfo, 0, sizeof(SysSoftwareVersionInfo));
    settingConfigRead(g_cfgTree, "version");

    m_versionInfo.softwareVersion = g_make_svn_revision;
    if (m_versionInfo.logoVersion == 0)
        m_versionInfo.logoVersion = g_make_svn_revision;
    if (m_versionInfo.settingVersion == 0)
        m_versionInfo.settingVersion = g_make_svn_revision;
    return ;
}

void upgrade_version_write(unsigned int pIndex, int ver)
{
    switch(pIndex) {
    case SOFTWARE_VERSION:
        if(m_versionInfo.softwareVersion == ver)
            return;
        m_versionInfo.softwareVersion = ver;
        break;
    case LOGO_VERSION:
        m_versionInfo.logoVersion = ver;
        break;
    case SETTING_VERSION:
        m_versionInfo.settingVersion = ver;
        break;
    case TEMPLATE_VERSION:
        if(m_versionInfo.templateVersion == ver)
            return;
        m_versionInfo.templateVersion = ver;
        break;
    case UPGRADE_FAILURE_NUM:
        if(m_versionInfo.failureNum == ver)
            return;
        m_versionInfo.failureNum = ver;
        break;
#ifdef INCLUDE_LITTLESYSTEM
    case SOFTWARE_VERSION_BAK:
         if(m_versionInfo.softwareVersionBak == ver)
            return;
        m_versionInfo.softwareVersionBak = ver;
	break;
#endif
    default:
        return;
    }
    m_versionInfo.dirty = 1;

    if(strncmp(m_versionInfo.CFversion, g_make_build_CFversion, USER_LEN-1))
        IND_STRNCPY(m_versionInfo.CFversion, g_make_build_CFversion, USER_LEN-1);

    settingConfigWrite(g_cfgTree, "version");
    return;
}

int upgrade_version_read(unsigned int pIndex)
{
    int version = 0;

    switch(pIndex) {
    case SOFTWARE_VERSION:
        version = m_versionInfo.softwareVersion;
        break;
    case LOGO_VERSION:
        version = m_versionInfo.logoVersion;
        break;
    case SETTING_VERSION:
        version = m_versionInfo.settingVersion;
        break;
    case TEMPLATE_VERSION:
        version = m_versionInfo.templateVersion;
        break;
    case UPGRADE_FAILURE_NUM:
        version = m_versionInfo.failureNum;
        break;
#ifdef INCLUDE_LITTLESYSTEM
    case SOFTWARE_VERSION_BAK:
        version = m_versionInfo.softwareVersionBak;
        break;
#endif
    default:
        version = 0;
        break;
    }

    return version;
}

int get_upgrade_version(char *buf)
{
#ifdef ANDROID
    sprintf(buf, "%s", StbInfo::STB::Version::Version());
    return 0;
#else
    sprintf(buf, "%d.%d.%d.0", m_versionInfo.softwareVersion, m_versionInfo.logoVersion, m_versionInfo.settingVersion);
    return 0;
#endif
}