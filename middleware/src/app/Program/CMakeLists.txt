
LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/Program.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ProgramAssertions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ProgramChannel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ProgramVOD.cpp
    ${CMAKE_CURRENT_LIST_DIR}/VodSource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ProgramParser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ProgramList.cpp
    ${CMAKE_CURRENT_LIST_DIR}/AdvertisementList.cpp
    )

LIST (APPEND INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}
    )

include (${CMAKE_CURRENT_LIST_DIR}/${BUILD_ContextType}/CMakeLists.txt)

