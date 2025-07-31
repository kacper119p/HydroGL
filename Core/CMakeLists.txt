set(LIBRARY_NAME Core)

file(GLOB_RECURSE SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/Src/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/Src/*.cpp")

file(GLOB_RECURSE HEADER_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/Src/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Src/*.h")

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HEADER_FILES})

add_library(${LIBRARY_NAME} STATIC ${SOURCE_FILES})

target_include_directories(${LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Src)

##Find OpenGL
find_package(OpenGL REQUIRED)

#find third party libraries
find_package(glm CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)

#Link OpenGL
target_link_libraries(${PROJECT_NAME} PRIVATE ${OPENGL_LIBRARIES})

#link third party libraries
target_link_libraries(${PROJECT_NAME} PUBLIC glm::glm)
target_link_libraries(${LIBRARY_NAME} PRIVATE glad)
target_link_libraries(${LIBRARY_NAME} PRIVATE spdlog::spdlog)
target_link_libraries(${LIBRARY_NAME} PRIVATE EnTT::EnTT)
target_link_libraries(${LIBRARY_NAME} PRIVATE boost)

#link project libraries
target_link_libraries(${LIBRARY_NAME} PRIVATE Platform)

target_include_directories(${LIBRARY_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR/Src}
        ${glad_SOURCE_DIR})

if (MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE NOMINMAX)
endif ()

target_compile_definitions(${LIBRARY_NAME} PRIVATE $<$<CONFIG:Debug>:DEBUG>)
target_compile_definitions(${LIBRARY_NAME} PRIVATE GLM_ENABLE_EXPERIMENTAL)