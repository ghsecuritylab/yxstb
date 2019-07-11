
LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/Widget.cpp
	${CMAKE_CURRENT_LIST_DIR}/Image.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Icon.cpp
    ${CMAKE_CURRENT_LIST_DIR}/PromptWidget.cpp
    ${CMAKE_CURRENT_LIST_DIR}/LogoWidget.cpp
    ${CMAKE_CURRENT_LIST_DIR}/PicSetWidget.cpp
    ${CMAKE_CURRENT_LIST_DIR}/VisualizationInfoWidget.cpp
    ${CMAKE_CURRENT_LIST_DIR}/MaintenancePageWidget.cpp
    ${CMAKE_CURRENT_LIST_DIR}/UsbInfoWidget.cpp
    )

LIST (APPEND INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}
    )

include (${CMAKE_CURRENT_LIST_DIR}/PlayWidgets/CMakeLists.txt)

