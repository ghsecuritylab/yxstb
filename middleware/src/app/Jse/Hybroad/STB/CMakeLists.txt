
LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/JseSTB.cpp
    )
    
include (${CMAKE_CURRENT_LIST_DIR}/Version/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Time/CMakeLists.txt)