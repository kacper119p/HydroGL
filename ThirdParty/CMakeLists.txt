find_package(OpenGL REQUIRED)

set(GLAD_DIR ${CMAKE_CURRENT_LIST_DIR}/Glad)
set(glad_SOURCE_DIR ${GLAD_DIR}/include CACHE INTERNAL "")
add_library(glad STATIC ${GLAD_DIR}/src/glad.c)
target_include_directories(glad PRIVATE ${GLAD_DIR}/include)


set(WGL_DIR ${CMAKE_CURRENT_LIST_DIR}/Wgl)
add_library(wgl INTERFACE)
target_include_directories(wgl INTERFACE ${WGL_DIR})


set(BOOST_DIR ${CMAKE_CURRENT_LIST_DIR}/Boost)
add_library(boost INTERFACE)
target_include_directories(boost INTERFACE ${BOOST_DIR})

set_target_properties(
        glad
        wgl
        boost
        PROPERTIES FOLDER "ThirdParty")