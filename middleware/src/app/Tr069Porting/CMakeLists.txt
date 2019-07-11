
IF (MODULE_Management STREQUAL "hybroad")
    include (${CMAKE_CURRENT_LIST_DIR}/Diagnostics/CMakeLists.txt)
    include (${CMAKE_CURRENT_LIST_DIR}/CPEConfig/CMakeLists.txt)
    include (${CMAKE_CURRENT_LIST_DIR}/ParameterMap/CMakeLists.txt)
    include (${CMAKE_CURRENT_LIST_DIR}/Statistics/CMakeLists.txt)
ENDIF (MODULE_Management STREQUAL "hybroad")

