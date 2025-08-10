#pragma once
#include <array>
#include <string>
#include <unordered_map>

#include "boost/functional/hash.hpp"
#include "Shader.h"
#include "ShaderStage.h"


namespace Core
{
    class ShaderBuilder
    {
    private:
        static constexpr size_t ShaderStagesArraySize = 6;

    private:
        struct ShaderStageDescription
        {
            std::string Path;
            ShaderStage Stage = ShaderStage::None;
        };

    public:
        struct ShaderCacheKey
        {
            std::array<ShaderStageDescription, ShaderStagesArraySize> ShaderStages{};

        public:
            explicit ShaderCacheKey(
                const std::array<ShaderStageDescription, ShaderStagesArraySize>& ShaderStages) : ShaderStages(
                ShaderStages)
            {
            }

            bool operator==(const ShaderCacheKey& Other) const
            {
                for (int i = 0; i < ShaderStagesArraySize; ++i)
                {
                    if (ShaderStages[i].Path != Other.ShaderStages[i].Path ||
                        ShaderStages[i].Stage != Other.ShaderStages[i].Stage)
                    {
                        return false;
                    }
                }
                return true;
            }
        };

        friend struct ::std::hash<::Core::ShaderBuilder::ShaderCacheKey>;

    public:
        static constexpr size_t RenderShaderStages = 5;

    private:
        static std::unordered_map<std::string, uint32_t> _shaderStageCache;
        static std::unordered_map<ShaderCacheKey, Shader> _shaderCache;

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

        static GLenum ShaderStageToGl(ShaderStage Stage);

    private:
        Shader CreateShader() const;

        Shader CreateComputeShader() const;
    };
} // namespace Core

template<>
struct ::std::hash<::Core::ShaderBuilder::ShaderCacheKey>
{
    std::size_t operator()(const ::Core::ShaderBuilder::ShaderCacheKey& Key) const noexcept
    {
        size_t hash = 0;
        for (int i = 0; i < ::Core::ShaderBuilder::ShaderStagesArraySize; ++i)
        {
            boost::hash_combine(hash, Key.ShaderStages[i].Path);
            boost::hash_combine(hash, Key.ShaderStages[i].Stage);
        }
        return hash;
    }
};
