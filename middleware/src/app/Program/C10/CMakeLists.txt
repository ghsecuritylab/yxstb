
LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/ProgramParserC10.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ProgramChannelC10.cpp
    )

MESSAGE (STATUS "_____________________________")
MESSAGE (STATUS "${CMAKE_CURRENT_LIST_DIR}")
MESSAGE (STATUS "____SHANGHAI_HD/SD___________")

IF (${CUSTOMER} STREQUAL "Shanghai_HD")
	include (${CMAKE_CURRENT_LIST_DIR}/ShanghaiHD/CMakeLists.txt)
ELSEIF (${CUSTOMER} STREQUAL "Shanghai_SD")
	include (${CMAKE_CURRENT_LIST_DIR}/ShanghaiSD/CMakeLists.txt)
ENDIF (${CUSTOMER} STREQUAL "Shanghai_HD")

LIST (APPEND INCLUDES
	${CMAKE_CURRENT_LIST_DIR}
    )



