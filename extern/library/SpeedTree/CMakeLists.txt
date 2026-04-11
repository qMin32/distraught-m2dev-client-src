add_library(SpeedTree STATIC IMPORTED GLOBAL)

set_target_properties(SpeedTree PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/extern/include"
)

set_target_properties(SpeedTree PROPERTIES
    IMPORTED_LOCATION_DEBUG   "${CMAKE_CURRENT_LIST_DIR}/speedtree_staticd.lib"
    IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_LIST_DIR}/speedtree_static.lib"
    IMPORTED_LOCATION_RELWITHDEBINFO "${CMAKE_CURRENT_LIST_DIR}/speedtree_static.lib"
)
