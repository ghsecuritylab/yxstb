
include (${CMAKE_CURRENT_LIST_DIR}/mad/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/independs/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/stream/CMakeLists.txt)

LIST (APPEND INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}
    )
