
LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/ProgramChannelC20.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ProgramParserC20.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ChannelListHttpCall.cpp
    ${CMAKE_CURRENT_LIST_DIR}/HttpInit.cpp
    )

IF (MODULE_DVBS_Tuner)
    LIST (APPEND IPTV_CXX_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/ProgramChannelDvb.cpp
        )
ENDIF (MODULE_DVBS_Tuner)

include (${CMAKE_CURRENT_LIST_DIR}/PPV/CMakeLists.txt)

LIST (APPEND INCLUDES
	${CMAKE_CURRENT_LIST_DIR}
    )


