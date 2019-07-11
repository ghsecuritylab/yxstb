
LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/Setting.cpp
    ${CMAKE_CURRENT_LIST_DIR}/AppSetting.cpp
    ${CMAKE_CURRENT_LIST_DIR}/SysSetting.cpp
    ${CMAKE_CURRENT_LIST_DIR}/VersionSetting.cpp
    ${CMAKE_CURRENT_LIST_DIR}/FrequenceSetting.cpp
    ${CMAKE_CURRENT_LIST_DIR}/SettingApi.cpp
    ${CMAKE_CURRENT_LIST_DIR}/SettingDigest.cpp
    ${CMAKE_CURRENT_LIST_DIR}/SettingModule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/SettingModuleNetwork.cpp
    ${CMAKE_CURRENT_LIST_DIR}/SettingModulePlayer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/SettingModuleUpgrade.cpp
    ${CMAKE_CURRENT_LIST_DIR}/SettingModuleApp.cpp
    ${CMAKE_CURRENT_LIST_DIR}/SettingListener.cpp
    )

IF (MODULE_Management STREQUAL "hybroad")
    LIST (APPEND IPTV_CXX_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/Tr069Setting.cpp
        ${CMAKE_CURRENT_LIST_DIR}/SettingModuleTr069.cpp
        )
ELSEIF (MODULE_Management STREQUAL "hybroad")
    LIST (APPEND IPTV_CXX_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/SettingModuleTr069.cpp
        )
ENDIF (MODULE_Management STREQUAL "hybroad")

IF (MODULE_DVBS_Tuner)
    LIST (APPEND IPTV_CXX_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/SettingModuleDvb.cpp
        )
ENDIF (MODULE_DVBS_Tuner)


IF (MODULE_CacheConfig)
    LIST (APPEND IPTV_CXX_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/CacheSetting.cpp
        )
ENDIF (MODULE_CacheConfig)


IF (BUILD_BakSetting)
    LIST (APPEND IPTV_CXX_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/BakSetting.cpp
        )
ENDIF (BUILD_BakSetting)

IF (BUILD_Android)
IF (STBTYPE STREQUAL "EC6106V8H")
    LIST (APPEND IPTV_CXX_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/AndroidSetting.cpp
    )
ENDIF (STBTYPE STREQUAL "EC6106V8H")    
ENDIF (BUILD_Android)

LIST (APPEND INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}
    )

include (${CMAKE_CURRENT_LIST_DIR}/Transform/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/TransformWifiConfig/CMakeLists.txt)

