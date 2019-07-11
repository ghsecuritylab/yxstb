
IF (BUILD_Android)
    return ()
ENDIF (BUILD_Android)

LIST (APPEND IPTV_C_SOURCES
    main.c
    )

LIST (APPEND LIBRARIES
    cairo
    freetype
    xml2
    oping
    )

foreach (LDS ${LIBS_DIRECTORIES})
    # MESSAGE (STATUS "LDS: ${LDS}")
    link_directories (${LDS})
endforeach (LDS)

set (TARGET_NAME iptv_B200.elf)

add_executable (${TARGET_NAME} ${IPTV_C_SOURCES})
add_dependencies (${TARGET_NAME} middleSoftwareA takinAgentA takinAgentN tr069)
 
target_link_libraries (${TARGET_NAME} -Wl,-whole-archive)
    foreach (LIBS ${LIBRARIES})
        # MESSAGE (STATUS "LIBS: ${LIBS}")
        target_link_libraries (${TARGET_NAME} ${LIBS})
    endforeach (LIBS)

    IF (BUILD_Support_LittleSystem)
        target_link_libraries (${TARGET_NAME} middleSoftwareS middleInitA takinAgentA)
    ELSE (BUILD_Support_LittleSystem)
        target_link_libraries (${TARGET_NAME} middleSoftwareA middleInitA takinAgentA)
    ENDIF (BUILD_Support_LittleSystem)
    IF (BUILD_BrowserIndependence)
        add_dependencies(${TARGET_NAME} jseglue_ipcA)
        target_link_libraries (${TARGET_NAME} jseglue_ipcA)
    ENDIF (BUILD_BrowserIndependence)
target_link_libraries (${TARGET_NAME} -Wl,-no-whole-archive)

foreach (LIBS ${PLATFORM_LINKFLAGS})
    # MESSAGE (STATUS "LIBS: ${LIBS}")
    target_link_libraries (${TARGET_NAME} ${LIBS})
endforeach (LIBS)

