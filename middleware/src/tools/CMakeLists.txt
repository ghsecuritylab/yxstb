
LIST (APPEND IPTV_CXX_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/ToolsAssertions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ConfigFileParser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/OSOpenSSL.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/UtilityTools.c
    ${CMAKE_CURRENT_LIST_DIR}/osex_net.c
    ${CMAKE_CURRENT_LIST_DIR}/charConvert.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cryptoFunc.cpp
	)

LIST (APPEND INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}
    )


include (${CMAKE_CURRENT_LIST_DIR}/json/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/mid/CMakeLists.txt)
