file(GLOB_RECURSE SOURCE_FILES
        *.c
        *.cpp)

file(GLOB_RECURSE HEADER_FILES
        *.h
        *.hpp)

add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})

##Find OpenGL
find_package(OpenGL REQUIRED)

#Find Vcpkg packages
find_package(imgui CONFIG REQUIRED)
find_package(imguizmo CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(Tracy CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

#Link OpenGL
target_link_libraries(${PROJECT_NAME} PRIVATE ${OPENGL_LIBRARIES})

#Link Vcpkg packages
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)
target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)
target_link_libraries(${PROJECT_NAME} PRIVATE Tracy::TracyClient)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog)
target_link_libraries(${PROJECT_NAME} PRIVATE imgui::imgui)
target_link_libraries(${PROJECT_NAME} PRIVATE imguizmo::imguizmo)

#Link local third party packages
target_link_libraries(${PROJECT_NAME} PRIVATE glad)

#link project libraries
target_link_libraries(${PROJECT_NAME} PRIVATE Shaders)


target_compile_definitions(${PROJECT_NAME} PRIVATE GLFW_INCLUDE_NONE)
target_compile_definitions(${PROJECT_NAME} PRIVATE LIBRARY_SUFFIX="")

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
        ${glad_SOURCE_DIR})