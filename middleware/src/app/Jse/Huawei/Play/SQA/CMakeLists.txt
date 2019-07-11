
LIST (APPEND IPTV_CXX_SOURCES
${CMAKE_CURRENT_LIST_DIR}/JseHWSQA.cpp
)
  
include (${CMAKE_CURRENT_LIST_DIR}/Buf/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Ret/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Tools/CMakeLists.txt)