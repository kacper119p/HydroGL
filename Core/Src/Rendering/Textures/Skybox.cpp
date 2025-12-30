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
        uint32_t width;
        uint32_t height;
        result._cubeMap = DdsLoader::LoadDds(SkyboxTexturePath, width, height);

        const GlTextureUnique iblTexture(DdsLoader::LoadDds(IblTexturePath));

        CHECK(width == height, "Skybox requires square faces!");

        result._irradianceMap = CubeMapGenerator::IrradianceMapFromSkybox(iblTexture.Get());
        result._prefilterMap = CubeMapGenerator::PrefilterMapFromSkybox(iblTexture.Get(), width);

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
