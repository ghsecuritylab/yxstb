
FILE( GLOB_RECURSE pay_shell_b200_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}		
	${CMAKE_CURRENT_SOURCE_DIR}/PayShellAssertions.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/PayShell.c
	)

include_directories(		
	${PROJECT_SOURCE_DIR}/EC2108_C27/include
	${PROJECT_SOURCE_DIR}/EC2108_C27/include/middle
	)
	
add_library( pay_shell_b200 STATIC ${pay_shell_b200_SRCS})

