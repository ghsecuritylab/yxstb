
LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/TempFile.cpp
    ${CMAKE_CURRENT_LIST_DIR}/PlatBuffer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RingBuffer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DataSource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DataSink.cpp
    ${CMAKE_CURRENT_LIST_DIR}/HttpDataSource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/FileStreamDataSource.cpp
    )

IF (MODULE_DVBS_Tuner)
    LIST (APPEND IPTV_CXX_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/DvbsDataSource.cpp
        )
ENDIF (MODULE_DVBS_Tuner)

LIST (APPEND INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}
    )



