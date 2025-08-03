#pragma once
#include <array>
#include <string>
#include <unordered_map>

#include "Shader.h"
#include "ShaderStage.h"



namespace Core
{
    class ShaderBuilder
    {
    private:
        struct ShaderStageDescription
        {
            std::string Path;
            ShaderStage Stage = ShaderStage::None;
        };

    public:
        static constexpr size_t ShaderStagesArraySize = 6;
        static constexpr size_t RenderShaderStages = 5;

    private:
        static std::unordered_map<std::string, uint32_t> ShaderStages;

    private:
        std::array<ShaderStageDescription, ShaderStagesArraySize> _shaderStages{};

    public:
        ShaderBuilder& SetStage(std::string&& Path, const ShaderStage Stage)
        {
            const uint8_t index = static_cast<uint8_t>(Stage);
            _shaderStages[index].Path = std::move(Path);
            _shaderStages[index].Stage = Stage;
            return *this;
        }

        ShaderBuilder& SetStage(const std::string& Path, const ShaderStage Stage)
        {
            const uint8_t index = static_cast<uint8_t>(Stage);
            _shaderStages[index].Path = Path;
            _shaderStages[index].Stage = Stage;
            return *this;
        }

        Shader Build() const;

    private:
        static uint32_t CompileShaderStage(const std::string& FilePath, GLenum Type);

        static uint32_t GetShaderStage(const std::string& FilePath, GLenum Type);

        static GLenum ShaderStageToGL(ShaderStage Stage);

    private:
        Shader CreateShader() const;

        Shader CreateComputeShader() const;
    };
}
