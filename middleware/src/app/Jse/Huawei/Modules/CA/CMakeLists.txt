
LIST (APPEND IPTV_CXX_SOURCES
${CMAKE_CURRENT_LIST_DIR}/JseHWCA.cpp
)

IF (MODULE_CA_TYPE STREQUAL "VMCA")
   include (${CMAKE_CURRENT_LIST_DIR}/Verimatrix/CMakeLists.txt)
ENDIF (MODULE_CA_TYPE STREQUAL "VMCA")

include (${CMAKE_CURRENT_LIST_DIR}/ICAS/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Irdeto/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Nagra/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/SecureMedia/CMakeLists.txt)


