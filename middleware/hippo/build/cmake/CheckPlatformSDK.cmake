####no need this configure. removed by teddy.

#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../SDK/static_libs)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../SDK/shared_libs)
#set(ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../SDK/static_libs)
#set(LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../SDK/shared_libs)
#####removed end.

add_definitions(-DOS_LINUX )
add_definitions(-Werror )


#message( STATUS, LIBRARY_OUTPUT_DIRECTORY=${LIBRARY_OUTPUT_DIRECTORY} )
#message( STATUS, RUNTIME_OUTPUT_DIRECTORY= ${CMAKE_CURRENT_BINARY_DIR} )
#message( STATUS, RUNTIME_OUTPUT_DIRECTORY= ${ARCHIVE_OUTPUT_DIRECTORY} )
