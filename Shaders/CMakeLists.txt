file(GLOB_RECURSE SOURCE_FILES
        *.c
        *.cpp)

file(GLOB_RECURSE HEADER_FILES
        *.h
        *.hpp)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HEADER_FILES})

add_library(Shaders STATIC ${SOURCE_FILES} ${ASSETS_FILES})