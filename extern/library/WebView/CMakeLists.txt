add_library(WebView STATIC IMPORTED GLOBAL)

set_target_properties(WebView PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/extern/include"
    IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/WebView2LoaderStatic.lib"
)
