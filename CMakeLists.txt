
LIST (APPEND DEFINITIONS 
    "-D${CUSTOMER}"
    "-D${STBTYPE}"
    "-D${CUSTOMER}_${STBTYPE}"
    "-DCUSTOMER=\"${CUSTOMER}\""
    "-DSTBTYPE=\"${STBTYPE}\""
    "-D${PLATFORM}"
    "-DPLATFORM_HOME=\"${PLATFORM_HOME}\""
    "-D_HW_BASE_VER_=59"
    )
    
IF (STBTYPE STREQUAL "EC6106V8H")
    LIST (APPEND DEFINITIONS
        "-DNEW_ANDROID_SETTING"
        "-DEC6106V8_TEST"
    )
ENDIF (STBTYPE STREQUAL "EC6106V8H") 

LIST (APPEND LIBRARIES
    ssl
    )

foreach (DEF ${PLATFORM_DEFINITIONS})
    add_definitions (${DEF})
endforeach (DEF)

add_subdirectory (${CMAKE_CURRENT_LIST_DIR}/isc-dhcp)
add_subdirectory (${CMAKE_CURRENT_LIST_DIR}/iptv)
add_subdirectory (${CMAKE_CURRENT_LIST_DIR}/ppp)
add_subdirectory (${CMAKE_CURRENT_LIST_DIR}/sqm)
add_subdirectory (${CMAKE_CURRENT_LIST_DIR}/independ_browser)
add_subdirectory (${CMAKE_CURRENT_LIST_DIR}/little_system)
add_subdirectory (${CMAKE_CURRENT_LIST_DIR}/middleware)
add_subdirectory (${CMAKE_CURRENT_LIST_DIR}/monitor)
add_subdirectory (${CMAKE_CURRENT_LIST_DIR}/net_check)
add_subdirectory (${CMAKE_CURRENT_LIST_DIR}/tr069)
add_subdirectory (${CMAKE_CURRENT_LIST_DIR}/android)
add_subdirectory (${CMAKE_CURRENT_LIST_DIR}/net_manager)

