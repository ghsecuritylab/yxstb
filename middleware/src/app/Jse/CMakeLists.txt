
LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/JseCall.cpp
    ${CMAKE_CURRENT_LIST_DIR}/JseFunctionCall.cpp
    ${CMAKE_CURRENT_LIST_DIR}/JseGroupCall.cpp
    ${CMAKE_CURRENT_LIST_DIR}/JseRoot.cpp
    ${CMAKE_CURRENT_LIST_DIR}/JseAssertions.cpp
    )

LIST (APPEND INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}
    )

include (${CMAKE_CURRENT_LIST_DIR}/Huawei/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Hybroad/CMakeLists.txt)
