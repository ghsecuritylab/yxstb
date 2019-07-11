
LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/JseHWNetwork.cpp
    )
    
include (${CMAKE_CURRENT_LIST_DIR}/DHCP/CMakeLists.txt)  
include (${CMAKE_CURRENT_LIST_DIR}/PPPOE/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Static/CMakeLists.txt)    