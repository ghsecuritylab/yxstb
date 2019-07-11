
LIST (APPEND IPTV_CXX_SOURCES
	${CMAKE_CURRENT_LIST_DIR}/UpgradeData.cpp
	${CMAKE_CURRENT_LIST_DIR}/UpgradeSource.cpp
	${CMAKE_CURRENT_LIST_DIR}/UpgradeChecker.cpp
	${CMAKE_CURRENT_LIST_DIR}/UpgradeReceiver.cpp
	${CMAKE_CURRENT_LIST_DIR}/UpgradeBurner.cpp
	${CMAKE_CURRENT_LIST_DIR}/UpgradeManager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/UpgradeAssertions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/UpgradeWidget.cpp
)

LIST (APPEND INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}
    )

include (${CMAKE_CURRENT_LIST_DIR}/BootImages/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/IP/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Template/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/UDisk/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Monitor/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Customer/CMakeLists.txt)




