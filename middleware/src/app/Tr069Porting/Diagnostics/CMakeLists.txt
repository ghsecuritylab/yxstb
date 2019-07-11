
IF (MODULE_Management STREQUAL "hybroad")

    LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/Tr069PlayInfo.cpp
    )
    LIST (APPEND INCLUDES
        ${CMAKE_CURRENT_LIST_DIR}/
    )
ENDIF (MODULE_Management STREQUAL "hybroad")

