add_library(DirectX INTERFACE)
target_include_directories(DirectX INTERFACE "${CMAKE_SOURCE_DIR}/extern/include")
target_link_libraries(DirectX INTERFACE "${CMAKE_CURRENT_LIST_DIR}/d3d9.lib" "${CMAKE_CURRENT_LIST_DIR}/d3dx9.lib" "${CMAKE_CURRENT_LIST_DIR}/dinput8.lib" "${CMAKE_CURRENT_LIST_DIR}/dxguid.lib")
