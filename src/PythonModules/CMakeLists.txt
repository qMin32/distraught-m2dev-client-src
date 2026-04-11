file(GLOB_RECURSE FILE_SOURCES CONFIGURE_DEPENDS "*.h" "*.c" "*.cpp")

add_library(PythonModules STATIC ${FILE_SOURCES})
GroupSourcesByFolder(PythonModules)
