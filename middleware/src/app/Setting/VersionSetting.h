#ifndef VersionSetting_h
#define VersionSetting_h

#ifdef __cplusplus
extern "C" {
#endif

void versionSettingInit(void);
void upgrade_version_write(unsigned int pIndex, int ver);
int upgrade_version_read(unsigned int pIndex);
int get_upgrade_version(char *buf);

#ifdef __cplusplus
}
#endif

#endif //VersionSetting_h