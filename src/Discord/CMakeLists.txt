file(GLOB_RECURSE FILE_SOURCES "*.h" "*.c" "*.cpp")

add_library(Discord STATIC ${FILE_SOURCES})

# target_link_libraries(Discord 
# )

GroupSourcesByFolder(Discord)
