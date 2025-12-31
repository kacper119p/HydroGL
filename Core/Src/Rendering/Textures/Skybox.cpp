#include "Skybox.h"


#include "CubeMapGenerator.h"
#include "Debug.h"
#include "Rendering/Shaders/Shader.h"
#include "Rendering/Shaders/ShaderBuilder.h"
#include "Rendering/Textures/DDSLoader.h"
#include "glad/glad.h"

namespace Core
{
    Skybox Skybox::CreateSkybox(const std::string_view SkyboxTexturePath, const std::string_view IblTexturePath)
    {
        Skybox result;

        uint32_t skyboxWidth;
        uint32_t skyboxHeight;
        result._cubeMap = DdsLoader::LoadDds(SkyboxTexturePath, skyboxWidth, skyboxHeight);
        CHECK(skyboxWidth == skyboxHeight, "Skybox requires square faces!");

        uint32_t iblWidth;
        uint32_t iblHeight;
        const GlTextureUnique iblTexture{DdsLoader::LoadDds(IblTexturePath, iblWidth, iblHeight)};
        CHECK(iblWidth == iblHeight, "IBL texture requires square faces!");


        result._irradianceMap = CubeMapGenerator::IrradianceMapFromSkybox(iblTexture.Get());
        result._prefilterMap = CubeMapGenerator::PrefilterMapFromSkybox(iblTexture.Get(), iblWidth);

        result._environmentColor = CubeMapGenerator::GetEnvironmentColorFromIrradianceMap(result._irradianceMap);

        return result;
    }

    void Skybox::DestroySkybox(const Skybox& Skybox)
    {
        glDeleteTextures(1, &Skybox._cubeMap);
        glDeleteTextures(1, &Skybox._irradianceMap);
        glDeleteTextures(1, &Skybox._prefilterMap);
    }
}
