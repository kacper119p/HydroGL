#pragma once
#include <glm/vec2.hpp>

#include "Rendering/Shaders/Shader.h"


namespace Core
{
    /**
     * @brief Class used to apply bloom effect to rendered scene.
     */
    class BloomPostprocessingEffect final
    {
    private:
        struct BloomMip
        {
            glm::vec2 Size;
            unsigned int Texture;
        };

    private:
        static constexpr size_t MipCount = 5;

        glm::uvec2 _resolution;
        uint32_t _frameBuffer = 0;
        uint32_t _prefilteredColor = 0;
        BloomMip _bloomMips[MipCount];

        Shader _prefilterShader;
        Shader _downSampleShader;
        Shader _upsampleShader;
        Shader _outputShader;

    public:
        explicit BloomPostprocessingEffect(glm::uvec2 Resolution);

    public:
        virtual ~BloomPostprocessingEffect();

    public:
        [[nodiscard]] uint32_t GetPrefilteredColor() const
        {
            return _prefilteredColor;
        }

        void Render(uint32_t SceneColorTexture);
    };

}
