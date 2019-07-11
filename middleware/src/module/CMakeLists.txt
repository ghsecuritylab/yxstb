
include (${CMAKE_CURRENT_LIST_DIR}/dns/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/ntp/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/play/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/ca/CMakeLists.txt)

LIST (APPEND INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/ca
    )
