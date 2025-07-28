set(LIBRARY_NAME Platform)

if (WIN32)
    file(GLOB_RECURSE SOURCE_FILES
            "${CMAKE_CURRENT_SOURCE_DIR}/Windows/*.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/Windows/*.cpp")

    file(GLOB_RECURSE HEADER_FILES
            "${CMAKE_CURRENT_SOURCE_DIR}/Windows/*.hpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/Windows/*.h")
endif ()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HEADER_FILES})

add_library(${LIBRARY_NAME} STATIC ${SOURCE_FILES})

if (WIN32)
    target_include_directories(${LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Windows)
endif ()

##Find OpenGL
find_package(OpenGL REQUIRED)

#find third party libraries
find_package(spdlog CONFIG REQUIRED)

#Link OpenGL
target_link_libraries(${PROJECT_NAME} PRIVATE ${OPENGL_LIBRARIES})

#link third party libraries
target_link_libraries(${LIBRARY_NAME} PRIVATE glad)
target_link_libraries(${LIBRARY_NAME} PRIVATE wgl)
target_link_libraries(${LIBRARY_NAME} PRIVATE spdlog::spdlog)

target_include_directories(${LIBRARY_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
        ${glad_SOURCE_DIR})

#Disable console for Release builds
if (MSVC)
    if (CMAKE_BUILD_TYPE STREQUAL "Release")
        set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
    else ()
        set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
    endif ()

    target_compile_definitions(${LIBRARY_NAME} PRIVATE NOMINMAX)
endif ()


target_compile_definitions(${LIBRARY_NAME} PRIVATE $<$<CONFIG:Debug>:DEBUG>)