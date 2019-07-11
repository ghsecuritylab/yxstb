
LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/Message.cpp
    ${CMAKE_CURRENT_LIST_DIR}/MessageQueue.cpp
    ${CMAKE_CURRENT_LIST_DIR}/MessageHandler.cpp
)

include_directories (
    ${CMAKE_CURRENT_LIST_DIR}
)
