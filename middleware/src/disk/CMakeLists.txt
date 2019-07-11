
LIST (APPEND INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}
    )

IF (MODULE_Disk)
    LIST (APPEND IPTV_C_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/disk_format.c
        ${CMAKE_CURRENT_LIST_DIR}/partition.c
        ${CMAKE_CURRENT_LIST_DIR}/disk_info.c
        ${CMAKE_CURRENT_LIST_DIR}/i386_sys_types.c
        )
    LIST (APPEND IPTV_CXX_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/DiskDev.cpp
        ${CMAKE_CURRENT_LIST_DIR}/disk_js_api.cpp
        ${CMAKE_CURRENT_LIST_DIR}/DiskPartition.cpp
        ${CMAKE_CURRENT_LIST_DIR}/DiskEvent.cpp
        ${CMAKE_CURRENT_LIST_DIR}/DiskManager.cpp
        )
ENDIF (MODULE_Disk)


