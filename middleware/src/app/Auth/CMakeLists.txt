

LIST (APPEND IPTV_CXX_SOURCES
	${CMAKE_CURRENT_LIST_DIR}/auth.cpp
	${CMAKE_CURRENT_LIST_DIR}/default_eds_roller.cpp
	${CMAKE_CURRENT_LIST_DIR}/default_url_checker.cpp
	${CMAKE_CURRENT_LIST_DIR}/concatenator_hw_auth.cpp
	${CMAKE_CURRENT_LIST_DIR}/concatenator_hw_logout.cpp
	${CMAKE_CURRENT_LIST_DIR}/concatenator_c20_auth.cpp
	${CMAKE_CURRENT_LIST_DIR}/concatenator_shanghai_auth.cpp
    )

LIST (APPEND INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}
    )


