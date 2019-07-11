IF (MODULE_Management STREQUAL "hybroad")
	IF (BUILD_Android)
		LIST (APPEND IPTV_CXX_SOURCES
			${CMAKE_CURRENT_LIST_DIR}/mgmtModuleAndroid.cpp
		)
	ELSE (BUILD_Android)
		LIST (APPEND IPTV_CXX_SOURCES
			${CMAKE_CURRENT_LIST_DIR}/mgmtModuleLinux.cpp
		)
	ENDIF (BUILD_Android)


	LIST (APPEND INCLUDES
		${CMAKE_CURRENT_LIST_DIR}
		)
	include (${CMAKE_CURRENT_LIST_DIR}/MgmtStbMonitor/CMakeLists.txt)
	include (${CMAKE_CURRENT_LIST_DIR}/MgmtTr069/CMakeLists.txt)

ENDIF (MODULE_Management STREQUAL "hybroad")



