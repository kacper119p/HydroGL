cmake_minimum_required(VERSION 3.31 FATAL_ERROR)
project(HydroGL VERSION 0.0.1)
set(CMAKE_CXX_STANDARD 23)

if (NOT WIN32)
    message(FATAL_ERROR "Project requires Windows system.")
endif ()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMake)

add_subdirectory(ThirdParty)
add_subdirectory(Shaders)
add_subdirectory(Core)
