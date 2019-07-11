
LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/Point.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Rect.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Region.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Canvas.cpp
    ${CMAKE_CURRENT_LIST_DIR}/View.cpp
    ${CMAKE_CURRENT_LIST_DIR}/OSDLayer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ViewPriv.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Window.cpp
    ${CMAKE_CURRENT_LIST_DIR}/StandardScreen.cpp
    ${CMAKE_CURRENT_LIST_DIR}/StandardViewGroup.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ViewAssertions.cpp
    )

LIST (APPEND INCLUDES
	${CMAKE_CURRENT_LIST_DIR}
)

include (${CMAKE_CURRENT_LIST_DIR}/Widgets/CMakeLists.txt)

