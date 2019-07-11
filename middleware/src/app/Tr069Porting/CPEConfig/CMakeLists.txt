

IF (MODULE_Management STREQUAL "hybroad")

        LIST (APPEND IPTV_C_SOURCES
            ${CMAKE_CURRENT_LIST_DIR}/tr069_port.c
            ${CMAKE_CURRENT_LIST_DIR}/TR069Assertions.cpp
            ${CMAKE_CURRENT_LIST_DIR}/tr069Itms.c
            )
ENDIF (MODULE_Management STREQUAL "hybroad")

LIST (APPEND INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}
    )


