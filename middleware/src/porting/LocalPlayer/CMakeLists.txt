
IF (MODULE_LocalPlayer)
    LIST (APPEND IPTV_CXX_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/CodeTransform.cpp
        ${CMAKE_CURRENT_LIST_DIR}/DirFileInfo.cpp
        ${CMAKE_CURRENT_LIST_DIR}/FileInfo.cpp
        ${CMAKE_CURRENT_LIST_DIR}/FileManager.cpp
        ${CMAKE_CURRENT_LIST_DIR}/localplayer_port.cpp
        ${CMAKE_CURRENT_LIST_DIR}/RegularFileInfo.cpp
        ${CMAKE_CURRENT_LIST_DIR}/HDplayer_api.cpp
        #${CMAKE_CURRENT_LIST_DIR}/VideoFileParser.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Mp3FileParser.cpp
        ${CMAKE_CURRENT_LIST_DIR}/AudioFileParserFactory.cpp
        )

    LIST (APPEND INCLUDES
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/include
        )
ENDIF (MODULE_LocalPlayer)



