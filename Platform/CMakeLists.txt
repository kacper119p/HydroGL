set(LIBRARY_NAME Platform)

file(GLOB_RECURSE SOURCE_FILES
        Src/*.c
        Src/*.cpp)

file(GLOB_RECURSE HEADER_FILES
        Src/*.h
        Src/*.hpp)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HEADER_FILES})

add_library(${LIBRARY_NAME} STATIC ${SOURCE_FILES})

target_include_directories(${LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)

find_package(spdlog CONFIG REQUIRED)

target_link_libraries(${LIBRARY_NAME} PRIVATE glad)
target_link_libraries(${LIBRARY_NAME} PRIVATE wgl)
target_link_libraries(${LIBRARY_NAME} PRIVATE spdlog::spdlog)

target_include_directories(${LIBRARY_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
        ${glad_SOURCE_DIR})

#Disable console for Release builds
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
else ()
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
endif ()

target_compile_definitions(${LIBRARY_NAME} PRIVATE NOMINMAX)
target_compile_definitions(${LIBRARY_NAME} PRIVATE $<$<CONFIG:Debug>:DEBUG>)