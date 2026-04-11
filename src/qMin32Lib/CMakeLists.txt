file(GLOB_RECURSE FILE_SOURCES "*.h" "*.c" "*.cpp")

add_library(qMin32Lib STATIC ${FILE_SOURCES})

target_link_libraries(qMin32Lib
	sodium
	mio
	freetype
)

GroupSourcesByFolder(qMin32Lib)
