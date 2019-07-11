
LIST (APPEND IPTV_CXX_SOURCES
    # ${CMAKE_CURRENT_LIST_DIR}/IPC.cpp 
    # ${CMAKE_CURRENT_LIST_DIR}/IPCServer.cpp 
    # ${CMAKE_CURRENT_LIST_DIR}/IPCCaller.cpp
    # ${CMAKE_CURRENT_LIST_DIR}/AgentShell.cpp
    # ${CMAKE_CURRENT_LIST_DIR}/IPCAssertions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/AgentHelper.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GlobalFunctions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/JVMThreadFuncs.cpp
    ${CMAKE_CURRENT_LIST_DIR}/MidwareRun.cpp
    ${CMAKE_CURRENT_LIST_DIR}/JseServer/IAuthenticationServer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/JseServer/IDeviceServer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/JseServer/IMediaPlayerServer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/JseServer/IUBankDeviceServer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/JseServer/IUtilityServer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/JseServer/IiPanelServer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/JseServer/IiPanelPageWidgetServer.cpp
)

LIST (APPEND INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/JseServer
    )


