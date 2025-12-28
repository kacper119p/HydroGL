find_package(OpenGL REQUIRED)

# ------------------------LOCAL LIBRARIES------------------------


set(GLAD_DIR ${CMAKE_CURRENT_LIST_DIR}/Glad)
set(glad_SOURCE_DIR ${GLAD_DIR}/include CACHE INTERNAL "")
add_library(glad STATIC ${GLAD_DIR}/src/glad.c)
target_include_directories(glad PRIVATE ${GLAD_DIR}/include)

if (WIN32)
    set(WGL_DIR ${CMAKE_CURRENT_LIST_DIR}/Wgl)
    add_library(wgl INTERFACE)
    target_include_directories(wgl INTERFACE ${WGL_DIR})
endif ()


set(BOOST_DIR ${CMAKE_CURRENT_LIST_DIR}/Boost)
add_library(boost INTERFACE)
target_include_directories(boost INTERFACE ${BOOST_DIR})

# -------------------------CPM LIBRARIES-------------------------

CPMAddPackage(
        NAME imgui
        GITHUB_REPOSITORY ocornut/imgui
        GIT_TAG docking
        DOWNLOAD_ONLY YES
)

if (imgui_ADDED)
    add_library(imgui STATIC
            "${imgui_SOURCE_DIR}/imgui.cpp"
            "${imgui_SOURCE_DIR}/imgui_demo.cpp"
            "${imgui_SOURCE_DIR}/imgui_draw.cpp"
            "${imgui_SOURCE_DIR}/imgui_tables.cpp"
            "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
            "${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp"
            "${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp"
    )
    target_include_directories(imgui PUBLIC "${imgui_SOURCE_DIR}")
endif ()

CPMAddPackage(
        NAME glm
        GITHUB_REPOSITORY g-truc/glm
        GIT_TAG 1.0.2
)

CPMAddPackage(
        NAME spdlog
        GITHUB_REPOSITORY gabime/spdlog
        GIT_TAG v1.16.0
)

CPMAddPackage(
        NAME tracy
        GITHUB_REPOSITORY wolfpld/tracy
        GIT_TAG v0.13.1
        OPTIONS "TRACY_ENABLE ON"
)

CPMAddPackage(
        NAME EnTT
        GITHUB_REPOSITORY skypjack/entt
        GIT_TAG v3.16.0
)

CPMAddPackage(
        NAME assimp
        GITHUB_REPOSITORY assimp/assimp
        GIT_TAG v6.0.2
        OPTIONS
        "ASSIMP_INJECT_DEBUG_POSTFIX OFF"
        "BUILD_SHARED_LIBS OFF"
        "ASSIMP_NO_EXPORT ON"
        "ASSIMP_BUILD_ASSIMP_TOOLS OFF"
        "ASSIMP_BUILD_TESTS OFF"
        "ASSIMP_INSTALL_PDB OFF"
        "ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF"
        "ASSIMP_BUILD_OBJ_IMPORTER OFF"
        "ASSIMP_BUILD_GLTF_IMPORTER ON"
        "ASSIMP_BUILD_FBX_IMPORTER ON"
        "ASSIMP_BUILD_MD5_IMPORTER OFF"
        "ASSIMP_DOUBLE_PRECISION OFF"
)

CPMAddPackage(
        NAME JoltPhysics
        GITHUB_REPOSITORY jrouwe/JoltPhysics
        GIT_TAG v5.5.0
        SOURCE_SUBDIR "Build"
        OPTIONS
        "TARGET_HELLO_WORLD OFF"
        "TARGET_SAMPLES OFF"
        "TARGET_VIEWER OFF"
        "TARGET_UNIT_TESTS OFF"
        "TARGET_PERFORMANCE_TEST OFF"
        "CROSS_PLATFORM_DETERMINISTIC OFF"
)

CPMAddPackage(
        NAME GTest
        GITHUB_REPOSITORY google/googletest
        GIT_TAG v1.17.0
        OPTIONS
        "INSTALL_GTEST OFF"
        "gtest_force_shared_crt ON"
)

set_target_properties(
        glad
        boost
        imgui
        assimp
        glm
        spdlog
        EnTT
        Jolt
        PROPERTIES FOLDER "ThirdParty")

if (WIN32)
    set_target_properties(wgl PROPERTIES FOLDER "ThirdParty")
endif ()
