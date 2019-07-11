
LIST (APPEND IPTV_CXX_SOURCES

    ${CMAKE_CURRENT_LIST_DIR}/Tr069Device.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Tr069STBService.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Tr069Config.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Tr069GatewayInfo.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Tr069ManagementServer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Tr069Time.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Tr069Qos.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Tr069PmInfo.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Tr069WLAN.cpp
    )

include (${CMAKE_CURRENT_LIST_DIR}/X_CTC_IPTV/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/X_CU_STB/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/X_00E0FC/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/X_COMMON/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/UserInterface/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/STBDevice/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/DeviceInfo/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/LAN/CMakeLists.txt)