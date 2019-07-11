
LIST (APPEND IPTV_C_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/stream_port.c
    ${CMAKE_CURRENT_LIST_DIR}/record_port.c
    ${CMAKE_CURRENT_LIST_DIR}/dvbs_port.c
    ${CMAKE_CURRENT_LIST_DIR}/stream_port_file.c
    )

IF (${MODULE_SQA} STREQUAL "none")
    LIST (APPEND IPTV_C_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/fcc_port_dummy.c
        )
ENDIF (${MODULE_SQA} STREQUAL "none")

IF (MODULE_Management STREQUAL "none")
    LIST (APPEND IPTV_C_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/stream_port_dummy.c
        )
ENDIF (MODULE_Management STREQUAL "none")


