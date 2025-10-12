file(GLOB_RECURSE SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/Src/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/Src/*.cpp")

file(GLOB_RECURSE HEADER_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/Src/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Src/*.h")

add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})

#Find Vcpkg packages
find_package(imgui CONFIG REQUIRED)
find_package(imguizmo CONFIG REQUIRED)
find_package(Tracy CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

#Link Vcpkg packages
target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)
target_link_libraries(${PROJECT_NAME} PRIVATE Tracy::TracyClient)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog)
target_link_libraries(${PROJECT_NAME} PRIVATE imgui::imgui)

#Link local third party packages
target_link_libraries(${PROJECT_NAME} PRIVATE glad)

#link project libraries
target_link_libraries(${PROJECT_NAME} PRIVATE Platform)
target_link_libraries(${PROJECT_NAME} PRIVATE Core)

target_compile_definitions(${PROJECT_NAME} PRIVATE LIBRARY_SUFFIX="")

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Src)

if (MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE NOMINMAX)
endif ()

target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:DEBUG>)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_SOURCE_DIR}/Assets
        ${CMAKE_CURRENT_BINARY_DIR}/Assets)
