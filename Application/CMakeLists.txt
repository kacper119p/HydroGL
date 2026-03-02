file(GLOB_RECURSE SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/Src/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/Src/*.cpp")

file(GLOB_RECURSE HEADER_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/Src/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Src/*.h")

add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})

#Link third party libraries
target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)
target_link_libraries(${PROJECT_NAME} PRIVATE Tracy::TracyClient)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog)
target_link_libraries(${PROJECT_NAME} PRIVATE imgui)
target_link_libraries(${PROJECT_NAME} PRIVATE Jolt)
target_link_libraries(${PROJECT_NAME} PRIVATE glad)

#link project libraries
target_link_libraries(${PROJECT_NAME} PRIVATE Platform)
target_link_libraries(${PROJECT_NAME} PRIVATE Core)

target_compile_definitions(${PROJECT_NAME} PRIVATE LIBRARY_SUFFIX="")

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Src)

# Windows specific settings
if (MSVC)
    #Disable console for Release builds
    if (CMAKE_BUILD_TYPE STREQUAL "Release")
        set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
    else ()
        set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
    endif ()

    target_compile_definitions(${PROJECT_NAME} PRIVATE NOMINMAX)

    target_sources(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Windows/resources.rc")
endif ()

target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:DEBUG>)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_SOURCE_DIR}/Assets
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/Assets)
