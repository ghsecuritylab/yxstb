
LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/JseHWTools.cpp
    )
    
include (${CMAKE_CURRENT_LIST_DIR}/AES/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Encrypt/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/ArrayTimer/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Record/CMakeLists.txt)
