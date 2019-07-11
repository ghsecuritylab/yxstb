
LIST (APPEND INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}/${PLATFORM}/include_priv
    )

include (${CMAKE_CURRENT_LIST_DIR}/${PLATFORM}/CMakeLists.txt)
