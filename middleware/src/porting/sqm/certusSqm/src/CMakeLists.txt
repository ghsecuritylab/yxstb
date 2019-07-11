#### Eagle 2011.01.17 ####

set (SQM_TARGET_NAME sqm.elf)

list (APPEND SQM_SRC_LIST 
    certus_main.c
    )

foreach (LDS ${LIBS_DIRECTORIES})
    link_directories (${LDS})
endforeach (LDS)

add_executable (${SQM_TARGET_NAME} ${SQM_SRC_LIST})

IF (DEBUG_Debug_Version)
	set (LIB_TYPE debug)
ELSE (DEBUG_Debug_Version)
	set (LIB_TYPE release)
ENDIF (DEBUG_Debug_Version)

set (SQM_VER certus)

set (SQMPRO_PATH            ${PROJECT_TOP_DIR}/platform/${PLATFORM}/${LIBTYPE}/sqmbin/${MODULE_SQM_VER})
set (SQM_SHARED_LIBS_PATH   ${PROJECT_TOP_DIR}/platform/${PLATFORM}/${LIBTYPE}/SQM_lib/${MODULE_SQM_VER})
set (SQM_LIBS               ${SQM_SHARED_LIBS_PATH}/libpcap.so)



target_link_libraries (${SQM_TARGET_NAME} pthread rt m gcc pcap)

include_directories (
    ${CMAKE_CURRENT_LIST_DIR}/..
    ${PROJECT_TOP_DIR}/hybroad/middleware/src/include
    ${iptv_BINARY_DIR}
    )

