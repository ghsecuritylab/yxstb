
LIST (APPEND IPTV_CXX_SOURCES
	${CMAKE_CURRENT_LIST_DIR}/UltraPlayer.cpp
	${CMAKE_CURRENT_LIST_DIR}/UltraPlayerMultiple.cpp
	${CMAKE_CURRENT_LIST_DIR}/UltraPlayerVod.cpp
	${CMAKE_CURRENT_LIST_DIR}/UltraPlayerUtility.cpp
	${CMAKE_CURRENT_LIST_DIR}/UltraPlayerAssertions.cpp
	${CMAKE_CURRENT_LIST_DIR}/UltraPlayerVideo.cpp
	${CMAKE_CURRENT_LIST_DIR}/VideoBrowserPlayer.cpp
	${CMAKE_CURRENT_LIST_DIR}/VideoPlayerManager.cpp	
)

IF (MODULE_DVBS_Tuner)
    LIST (APPEND IPTV_CXX_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/UltraPlayerDvb.cpp
        )
ENDIF (MODULE_DVBS_Tuner)

include (${CMAKE_CURRENT_LIST_DIR}/Huawei/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/UltraPlayerStatistic/CMakeLists.txt)

LIST (APPEND INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}
    )


