

LIST (APPEND IPTV_CXX_SOURCES
	${CMAKE_CURRENT_LIST_DIR}/Keyboard.cpp
    )

IF (BUILD_Android)
    LIST (APPEND IPTV_CXX_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/KeyboardAndroid.cpp
        )
ELSEIF (BUILD_BrowserIndependence)
    LIST (APPEND IPTV_CXX_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/KeyboardBrowserIndepend.cpp
        )
ELSE (BUILD_Android)
    LIST (APPEND IPTV_CXX_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/KeyboardLinux.cpp
        )
ENDIF (BUILD_Android)

LIST (APPEND INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}
    )


