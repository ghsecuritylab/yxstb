
LIST (APPEND INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}/include/middle
    ${CMAKE_CURRENT_LIST_DIR}/include/porting
    ${CMAKE_CURRENT_LIST_DIR}/../independ_browser
    ${CMAKE_CURRENT_LIST_DIR}/../independ_browser/ipc
    ${CMAKE_CURRENT_LIST_DIR}/../tr069

    # ${CMAKE_CURRENT_LIST_DIR}/platform/include
    # ${CMAKE_CURRENT_LIST_DIR}/src/include
    # ${CMAKE_CURRENT_LIST_DIR}/src/include/porting
    # ${CMAKE_CURRENT_LIST_DIR}/src/include/middle
    # ${CMAKE_CURRENT_LIST_DIR}/src/app/BrowserBridge/
    # ${CMAKE_CURRENT_LIST_DIR}/src/app/Util/
    # ${CMAKE_CURRENT_LIST_DIR}/src/app/Message/
    # ${CMAKE_CURRENT_LIST_DIR}/src/app/Devices/
    # ${CMAKE_CURRENT_LIST_DIR}/src/app/Views/
    # ${CMAKE_CURRENT_LIST_DIR}/src/app/Views/Widgets/
    # ${CMAKE_CURRENT_LIST_DIR}/src/porting/tr069/
    # ${CMAKE_CURRENT_LIST_DIR}/src/tr069/

    ${TAKIN_SDK_PATH}/include/porting/core
    ${TAKIN_SDK_PATH}/include/base
    ${TAKIN_SDK_PATH}/include
    ${PLATFORM_SDK_PATH}/include
    ${PLATFORM_SDK_PATH}/include/sdk
    ${PLATFORM_SDK_PATH}/Hi3560E_V100R001C01SPC020/his_include
    ${iptv_BINARY_DIR}/third_party/include
    ${iptv_BINARY_DIR}/third_party/include/freetype2
    ${iptv_BINARY_DIR}/third_party/include/pixman
    ${iptv_BINARY_DIR}/third_party/include/curl
    ${iptv_BINARY_DIR}/third_party/include/cairo
    ${iptv_BINARY_DIR}/third_party/include/zlib
    ${iptv_BINARY_DIR}/third_party/include/jpeg
    ${iptv_BINARY_DIR}/third_party/include/png
    ${iptv_BINARY_DIR}
    )

include (${CMAKE_CURRENT_LIST_DIR}/hippo/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/platform/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/src/CMakeLists.txt)

LIST (REMOVE_DUPLICATES IPTV_C_SOURCES)
LIST (REMOVE_DUPLICATES IPTV_CXX_SOURCES)
LIST (REMOVE_DUPLICATES IPTV_INIT_SOURCES)
LIST (REMOVE_DUPLICATES DEFINITIONS)
LIST (REMOVE_DUPLICATES INCLUDES)
LIST (REMOVE_DUPLICATES PLATFORM_DEFINITIONS)
LIST (REMOVE_DUPLICATES PLATFORM_LINKFLAGS)
LIST (REMOVE_DUPLICATES LIBS_DIRECTORIES)
LIST (REMOVE_DUPLICATES BROWSERAGENT_SOURCES)

foreach (INC ${INCLUDES})
    # MESSAGE (STATUS "INCLUDE: ${INC}")
    include_directories (${INC})
endforeach (INC)

foreach (DEF ${DEFINITIONS})
    # MESSAGE (STATUS "MACRO: ${DEF}")
    add_definitions (${DEF})
endforeach (DEF)

foreach (LDS ${LIBS_DIRECTORIES})
    link_directories(${LDS})
endforeach (LDS ${LIBS_DIRECTORIES})

SET (TARGET_LIB_NAME_A middleSoftwareA)
SET (TARGET_INIT_LIB_NAME middleInitA)
SET (TARGET_BROWSER_AGENT_A takinAgentA)
SET (TARGET_BROWSER_AGENT_N takinAgentN)
SET (TARGET_ANDROID_LIB_NAME androidmw)


add_library (${TARGET_LIB_NAME_A} ${IPTV_C_SOURCES} ${IPTV_CXX_SOURCES}) # ${IPTV_INIT_SOURCES})
add_library (${TARGET_INIT_LIB_NAME} ${IPTV_INIT_SOURCES})
add_library (${TARGET_BROWSER_AGENT_A} ${BROWSERAGENT_SOURCES})
add_library (${TARGET_BROWSER_AGENT_N} ${BROWSERAGENT_SOURCES})
add_library (${TARGET_ANDROID_LIB_NAME} ${IPTV_C_SOURCES} ${IPTV_CXX_SOURCES} ${IPTV_INIT_SOURCES} ${BROWSERAGENT_SOURCES})

IF (NOT BUILD_Android)
    SET (TARGET_LIB_NAME_S middleSoftwareS)
    add_library (${TARGET_LIB_NAME_S} SHARED ${IPTV_C_SOURCES} ${IPTV_CXX_SOURCES}) # ${IPTV_INIT_SOURCES})
ENDIF (NOT BUILD_Android)

set_target_properties (${TARGET_BROWSER_AGENT_N} PROPERTIES 
    COMPILE_FLAGS "-DNONE_BROWSER"
    )

add_dependencies(${TARGET_ANDROID_LIB_NAME} platform_sdk)










