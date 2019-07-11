
LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/NativeHandler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/NativeHandlerAssertions.cpp
    )

LIST (APPEND INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}
    )

include (${CMAKE_CURRENT_LIST_DIR}/Huawei/CMakeLists.txt)



