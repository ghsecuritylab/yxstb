# cmake_minimum_required (VERSION 2.8.3)       

set (NETWORK_MODULE_DIR ${CMAKE_CURRENT_LIST_DIR})

include_directories (
    ${NETWORK_MODULE_DIR}
    ${NETWORK_MODULE_DIR}/Settings/
    ${NETWORK_MODULE_DIR}/StateHandler/
    ${NETWORK_MODULE_DIR}/DiagnoseTools/
)

list (APPEND NETWORK_SRC_LIST
    ${NETWORK_MODULE_DIR}/NetworkDummy.cpp          
    ${NETWORK_MODULE_DIR}/NetworkFunctions.cpp          
    ${NETWORK_MODULE_DIR}/NetworkAssertions.cpp          
    ${NETWORK_MODULE_DIR}/NetworkManager.cpp          
    ${NETWORK_MODULE_DIR}/NetworkCard.cpp          
    ${NETWORK_MODULE_DIR}/NetworkInterface.cpp          
    ${NETWORK_MODULE_DIR}/NetworkErrorCode.cpp          
    ${NETWORK_MODULE_DIR}/WiredNetworkCard.cpp          
    ${NETWORK_MODULE_DIR}/WirelessNetworkCard.cpp          
)

IF (BUILD_Android)
    list (APPEND NETWORK_SRC_LIST 
        ${NETWORK_MODULE_DIR}/NetworkInterfaceAndroid.cpp
        ${NETWORK_MODULE_DIR}/AndroidNetwork.cpp
    )
ELSE (BUILD_Android)    
    list (APPEND NETWORK_SRC_LIST 
        ${NETWORK_MODULE_DIR}/NetworkInterfaceLinux.cpp
    )
ENDIF (BUILD_Android)

include (${NETWORK_MODULE_DIR}/Settings/CMakeLists.txt)
include (${NETWORK_MODULE_DIR}/StateHandler/CMakeLists.txt)
include (${NETWORK_MODULE_DIR}/ErrorCodes/CMakeLists.txt)
include (${NETWORK_MODULE_DIR}/DiagnoseTools/CMakeLists.txt)

list (APPEND IPTV_CXX_SOURCES ${NETWORK_SRC_LIST})
