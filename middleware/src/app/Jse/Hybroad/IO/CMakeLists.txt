
LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/JseIO.cpp
    )
    
include (${CMAKE_CURRENT_LIST_DIR}/Analog/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Digital/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Margin/CMakeLists.txt)