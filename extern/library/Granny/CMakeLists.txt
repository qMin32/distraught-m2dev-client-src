add_library(Granny STATIC IMPORTED GLOBAL)

set_target_properties(Granny PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/extern/include"
    IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/granny2_static.lib"
)
