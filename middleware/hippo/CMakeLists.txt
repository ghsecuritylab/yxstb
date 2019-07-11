##################################################
#                                                #
# Main CMakeLists for Hippo Project.			 #
#                                                #
##################################################
#cmake_minimum_required (VERSION 2.6)
#project(Hippo CXX C)

LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/PlatformCommon/browser/takin/Hippo_HTakinActiveObj.cpp
    ${CMAKE_CURRENT_LIST_DIR}/PlatformCommon/browser/Hippo_HBrowserActiveObj.cpp
    ${CMAKE_CURRENT_LIST_DIR}/PlatformCommon/context/Hippo_Context.cpp
    ${CMAKE_CURRENT_LIST_DIR}/PlatformCommon/context/Hippo_ioctlMap.cpp
    ${CMAKE_CURRENT_LIST_DIR}/PlatformCommon/context/Hippo_CTCUBankDevice.cpp
    ${CMAKE_CURRENT_LIST_DIR}/PlatformCommon/utility/Hippo_HString.cpp
    # ${CMAKE_CURRENT_LIST_DIR}/PlatformCommon/utility/Hippo_HMap.cpp
    # ${CMAKE_CURRENT_LIST_DIR}/PlatformCommon/player/Hippo_PlayerNode.cpp
    ${CMAKE_CURRENT_LIST_DIR}/PlatformCommon/player/Hippo_MediaPlayer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/PlatformCommon/player/Hippo_Player.cpp
    # ${CMAKE_CURRENT_LIST_DIR}/PlatformCommon/player/Hippo_HPlayControler.cpp
    # ${CMAKE_CURRENT_LIST_DIR}/PlatformCommon/settings/Hippo_Setings.cpp
    # ${CMAKE_CURRENT_LIST_DIR}/CPP2C/Hippo_channel_api.cpp
    # ${CMAKE_CURRENT_LIST_DIR}/CPP2C/Hippo_media_api.cpp
    # ${CMAKE_CURRENT_LIST_DIR}/CPP2C/Hippo_ipanel_api.cpp
    ${CMAKE_CURRENT_LIST_DIR}/CPP2C/Hippo_api.cpp
    # ${CMAKE_CURRENT_LIST_DIR}/IPTVPlatformSpec/Huaweibase/Hippo_AdvPlayNodeHWBase.cpp
    ${CMAKE_CURRENT_LIST_DIR}/IPTVPlatformSpec/Huaweibase/Hippo_PlayerHWBase.cpp
    ${CMAKE_CURRENT_LIST_DIR}/IPTVPlatformSpec/Huaweibase/Hippo_ContextBase.cpp
    # ${CMAKE_CURRENT_LIST_DIR}/IPTVPlatformSpec/Huaweibase/Hippo_VodPlayNodeHWBase.cpp
    # ${CMAKE_CURRENT_LIST_DIR}/IPTVPlatformSpec/Huaweibase/Hippo_ChannelPlayerNodeHWBase.cpp
    # ${CMAKE_CURRENT_LIST_DIR}/test_stub/test_stub.cpp
    # ${CMAKE_CURRENT_LIST_DIR}/OSPlatformSpec/HippoBase/src/Hippo_ThreadGlobalData.cpp
    # ${CMAKE_CURRENT_LIST_DIR}/OSPlatformSpec/HippoBase/src/Hippo_HEventDispatcher.cpp
    # ${CMAKE_CURRENT_LIST_DIR}/OSPlatformSpec/HippoBase/src/Interface.cpp
    # ${CMAKE_CURRENT_LIST_DIR}/OSPlatformSpec/HippoBase/src/InterfaceDHCP.cpp
    # ${CMAKE_CURRENT_LIST_DIR}/OSPlatformSpec/HippoBase/src/Hippo_HTimer.cpp
    # ${CMAKE_CURRENT_LIST_DIR}/OSPlatformSpec/HippoBase/src/DeviceEthernet.cpp
    # ${CMAKE_CURRENT_LIST_DIR}/OSPlatformSpec/HippoBase/src/Hippo_HIPCMsgQueue.cpp
    ${CMAKE_CURRENT_LIST_DIR}/OSPlatformSpec/HippoBase/src/Hippo_HEvent.cpp
    # ${CMAKE_CURRENT_LIST_DIR}/OSPlatformSpec/HippoBase/src/InterfacePPPoE.cpp
    # ${CMAKE_CURRENT_LIST_DIR}/OSPlatformSpec/HippoBase/src/Hippo_HThreadTimers.cpp
    # ${CMAKE_CURRENT_LIST_DIR}/OSPlatformSpec/HippoBase/src/Hippo_HThread.cpp
    # ${CMAKE_CURRENT_LIST_DIR}/OSPlatformSpec/HippoBase/src/STBConfig.cpp
    # ${CMAKE_CURRENT_LIST_DIR}/OSPlatformSpec/HippoBase/src/DeviceWiFi.cpp
    # ${CMAKE_CURRENT_LIST_DIR}/OSPlatformSpec/HippoBase/src/Device.cpp
    # ${CMAKE_CURRENT_LIST_DIR}/OSPlatformSpec/linux/ThreadSpecificLinux.cpp
    # ${CMAKE_CURRENT_LIST_DIR}/OSPlatformSpec/linux/ThreadSharedTimerLinux.cpp
    ${CMAKE_CURRENT_LIST_DIR}/OSPlatformSpec/linux/Hippo_HEventBaseLinux.cpp
    ${CMAKE_CURRENT_LIST_DIR}/IPTVPlatformSpec/Huaweibase/${BUILD_ContextType}/Hippo_ContextHW${BUILD_ContextType}.cpp
    )


LIST (APPEND INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}/CPP2C
    ${CMAKE_CURRENT_LIST_DIR}/OSPlatformSpec/HippoBase/include
    ${CMAKE_CURRENT_LIST_DIR}/IPTVPlatformSpec/Huaweibase
    )

LIST (APPEND DEFINITIONS
    "-DOS_LINUX"
    "-DOS_LINUX_OS"
    "-DTAKIN_BROWSER"
    "-DOLD_ARCH"
    "-DSelected_HippoContext_Type=\"${BUILD_ContextType}\""
    )




