
IF (BUILD_Android)
    return ()
ENDIF (BUILD_Android)


include_directories (
    ${CMAKE_CURRENT_LIST_DIR}/../middleware/include/
    )

include (${CMAKE_CURRENT_LIST_DIR}/pppoe/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/pppd/CMakeLists.txt)

