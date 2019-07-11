
LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/JseHuawei.cpp
    )

include (${CMAKE_CURRENT_LIST_DIR}/Tools/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/STB/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/IO/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Network/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Modules/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Play/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Maintenance/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Business/CMakeLists.txt)