
LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/JseNetwork.cpp
    )
    
IF (MODULE_Wifi)    
    include (${CMAKE_CURRENT_LIST_DIR}/Wifi/CMakeLists.txt)
ENDIF (MODULE_Wifi)
    
include (${CMAKE_CURRENT_LIST_DIR}/Static/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/DHCP/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/PPPOE/CMakeLists.txt)