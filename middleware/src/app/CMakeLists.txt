
#FILE( GLOB_RECURSE app_b200_SRC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
#	${CMAKE_CURRENT_SOURCE_DIR}/*.c
#	${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
#	)

LIST (APPEND IPTV_C_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/app_epg_para.c
    ${CMAKE_CURRENT_LIST_DIR}/app_jse.c
    ${CMAKE_CURRENT_LIST_DIR}/app_pressuretest.c
    ${CMAKE_CURRENT_LIST_DIR}/app_sys.c
    ${CMAKE_CURRENT_LIST_DIR}/app_tool.c
    ${CMAKE_CURRENT_LIST_DIR}/jse.c
    )

LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/app_heartbit.cpp
    ${CMAKE_CURRENT_LIST_DIR}/app_reminder.cpp
	${CMAKE_CURRENT_LIST_DIR}/Assertions.cpp
	${CMAKE_CURRENT_LIST_DIR}/KeyDispatcher.cpp
	${CMAKE_CURRENT_LIST_DIR}/MainThread.cpp
	${CMAKE_CURRENT_LIST_DIR}/SystemManager.cpp
    )

IF (NOT MODULE_Management STREQUAL "none")
    LIST (APPEND IPTV_CXX_SOURCES
        )
ENDIF (NOT MODULE_Management STREQUAL "none")

LIST (APPEND INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}/
    )

include (${CMAKE_CURRENT_LIST_DIR}/Customer/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/DataStream/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Devices/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Message/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Util/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Log/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/BrowserBridge/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Upgrade/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Program/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/NativeHandler/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Views/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Player/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Setting/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Dvb/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Network/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Jse/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Tr069Porting/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Auth/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Keyboard/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Maintenance/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Business/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/mgmt/CMakeLists.txt)

IF (BUILD_Android)
    LIST (APPEND IPTV_CXX_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/IPTVMiddleware.cpp
    )
ENDIF (BUILD_Android)




