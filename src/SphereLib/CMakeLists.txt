file(GLOB_RECURSE FILE_SOURCES "*.h" "*.c" "*.cpp")

add_library(SphereLib STATIC ${FILE_SOURCES})

# target_link_libraries(SphereLib 
# )

GroupSourcesByFolder(SphereLib)
