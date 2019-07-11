IF (NOT BUILD_Android)
    return ()
ENDIF (NOT BUILD_Android)

IF (STBTYPE STREQUAL "EC6106V8H")

MACRO (RunNDKBuild)
    add_custom_target (run_ndk ALL
        COMMAND NDK_PROJECT_PATH=${CMAKE_CURRENT_LIST_DIR} WORKING_DIR=${iptv_BINARY_DIR} exec "${SCRIPT_PATH}/ndk_build.sh"
        VERBATIM
        )
    add_dependencies(run_ndk androidmw middleSoftwareA takinAgentA takinAgentN tr069)
ENDMACRO (RunNDKBuild)

RunNDKBuild()

add_custom_target(apk
    COMMAND exec "${SCRIPT_PATH}/make_apk.sh"
    WORKING_DIRECTORY ${iptv_BINARY_DIR}
    VERBATIM
    )

add_custom_target(otapackage
    COMMAND LAST_VERSION_PACKAGE=${SDKTAR_PATH}/${LAST_VERSION_PACKAGE} exec "${SCRIPT_PATH}/make_otapackage.sh"
    WORKING_DIRECTORY ${PLATFORM_SDK_PATH}
    #COMMENT ${SDKTAR_PATH}/${LAST_VERSION_PACKAGE}
    VERBATIM
    )

add_dependencies (apk run_ndk)
ENDIF (STBTYPE STREQUAL "EC6106V8H")
