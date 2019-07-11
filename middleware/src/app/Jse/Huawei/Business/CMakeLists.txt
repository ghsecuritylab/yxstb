
LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/JseHWBusiness.cpp
    )

include (${CMAKE_CURRENT_LIST_DIR}/Account/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Ad/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Channel/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Logo/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Resource/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Session/CMakeLists.txt)