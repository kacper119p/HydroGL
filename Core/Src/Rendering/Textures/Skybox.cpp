#include "Skybox.h"


#include "CubeMapGenerator.h"
#include "Debug.h"
#include "Rendering/Shaders/Shader.h"
#include "Rendering/Shaders/ShaderBuilder.h"
#include "Rendering/Textures/DDSLoader.h"
#include "glad/glad.h"

namespace Core
{
    Skybox Skybox::CreateSkybox(const std::string_view& FilePath)
    {
        Skybox result;
        uint32_t width;
        uint32_t height;
        result._cubeMap = DdsLoader::LoadDds(FilePath, width, height);

        CHECK_MESSAGE(width == height, "Skybox requires square faces!")

        result._irradianceMap = CubeMapGenerator::IrradianceMapFromSkybox(result._cubeMap);
        result._prefilterMap = CubeMapGenerator::PrefilterMapFromSkybox(result._cubeMap, width);

        return result;
    }

    void Skybox::DestroySkybox(const Skybox& Skybox)
    {
        glDeleteTextures(1, &Skybox._cubeMap);
        glDeleteTextures(1, &Skybox._irradianceMap);
        glDeleteTextures(1, &Skybox._prefilterMap);
    }
}
