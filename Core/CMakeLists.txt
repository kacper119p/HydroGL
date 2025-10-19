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
find_package(assimp CONFIG REQUIRED)
find_package(Jolt CONFIG REQUIRED)

#Link OpenGL
target_link_libraries(${LIBRARY_NAME} PRIVATE ${OPENGL_LIBRARIES})

#link third party libraries
target_link_libraries(${LIBRARY_NAME} PUBLIC Jolt::Jolt)
target_link_libraries(${LIBRARY_NAME} PUBLIC glm::glm)
target_link_libraries(${LIBRARY_NAME} PUBLIC EnTT::EnTT)
target_link_libraries(${LIBRARY_NAME} PUBLIC boost)
target_link_libraries(${LIBRARY_NAME} PRIVATE glad)
target_link_libraries(${LIBRARY_NAME} PRIVATE spdlog::spdlog)
target_link_libraries(${LIBRARY_NAME} PRIVATE assimp::assimp)

#link project libraries
target_link_libraries(${LIBRARY_NAME} PRIVATE Platform)

target_include_directories(${LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR/Src}
        ${glad_SOURCE_DIR})

if (MSVC)
    target_compile_definitions(${LIBRARY_NAME} PRIVATE NOMINMAX)
endif ()

target_compile_definitions(${LIBRARY_NAME} PRIVATE $<$<CONFIG:Debug>:DEBUG>)
target_compile_definitions(${LIBRARY_NAME} PRIVATE GLM_ENABLE_EXPERIMENTAL)

# --------------------------------TESTS--------------------------------

set(TESTS_NAME CoreTests)

find_package(GTest CONFIG REQUIRED)

enable_testing()

file(GLOB_RECURSE TEST_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/Test/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/Test/*.cpp")

file(GLOB_RECURSE TEST_HEADER_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/Test/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Test/*.h")

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TEST_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TEST_HEADER_FILES})

add_executable(${TESTS_NAME} ${TEST_HEADER_FILES} ${TEST_FILES})

target_include_directories(${TESTS_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Test)

target_link_libraries(${TESTS_NAME} PRIVATE ${LIBRARY_NAME})
target_link_libraries(${TESTS_NAME} PRIVATE GTest::gtest_main)
target_link_libraries(${TESTS_NAME} PRIVATE glm::glm)
target_link_libraries(${TESTS_NAME} PRIVATE glad)
target_link_libraries(${TESTS_NAME} PRIVATE spdlog::spdlog)
target_link_libraries(${TESTS_NAME} PRIVATE EnTT::EnTT)
target_link_libraries(${TESTS_NAME} PRIVATE boost)

if (MSVC)
    target_compile_definitions(${TESTS_NAME} PRIVATE NOMINMAX)
endif ()
target_compile_definitions(${TESTS_NAME} PRIVATE GLM_ENABLE_EXPERIMENTAL)

include(GoogleTest)
gtest_discover_tests(CoreTests)
