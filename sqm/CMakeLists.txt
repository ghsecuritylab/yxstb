
return()

LIST (APPEND SQM_SOURCES
    ${MODULE_SQM_VER}Sqm/sqm_main.c
    )

SET (SQM_TARGET_NAME sqm1.elf)

add_executable (${SQM_TARGET_NAME} ${SQM_SOURCES})

target_link_libraries (${SQM_TARGET_NAME} pthread rt m gcc pcap)

IF (DEBUG_Debug_Version)
    SET (LIB_TYPE debug)
ELSE (DEBUG_Debug_Version)
    SET (LIB_TYPE release)
ENDIF (DEBUG_Debug_Version)


