
LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/Resource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ResourceUser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ResourceManager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Device.cpp
    ${CMAKE_CURRENT_LIST_DIR}/FrontPanelDevice.cpp
    ${CMAKE_CURRENT_LIST_DIR}/LayerMixerDevice.cpp
    ${CMAKE_CURRENT_LIST_DIR}/NetworkDevice.cpp
    ${CMAKE_CURRENT_LIST_DIR}/LayerMixerDeviceZebra.cpp
    ${CMAKE_CURRENT_LIST_DIR}/TunerDevice.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DiskDevice.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Concurrent.cpp
    )

LIST (APPEND INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}
    )
