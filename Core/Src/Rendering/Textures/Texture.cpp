#include "Texture.h"

#include <filesystem>
#include <ranges>

#include "DDSLoader.h"

namespace Core
{
    std::unordered_map<std::string, Texture> Texture::_textureCache;

    Texture Texture::GetTexture(const std::string_view& Path)
    {
        if (const auto iterator = _textureCache.find(Path.data()); iterator != _textureCache.end())
        {
            return Texture(iterator->second);
        }

        uint32_t width;
        uint32_t height;
        uint32_t textureId = DdsLoader::LoadDds(Path, width, height);

        _textureCache.emplace(Path, textureId);
        return Texture(textureId);
    }

    bool Texture::DeleteTexture(const std::string_view& Path)
    {
        if (const auto iterator = _textureCache.find(Path.data()); iterator != _textureCache.end())
        {
            const uint32_t id = iterator->second.GetId();
            glDeleteTextures(1, &id);
            _textureCache.erase(iterator);
            return true;
        }
        return false;
    }

    bool Texture::DeleteTexture(const Texture& Texture)
    {
        for (const auto& [path, texture] : _textureCache)
        {
            if (texture == Texture)
            {
                const uint32_t id = texture.GetId();
                glDeleteTextures(1, &id);
                _textureCache.erase(path);
                return true;
            }
        }
        return false;
    }

    void Texture::DeleteAllTextures()
    {
        for (const auto& val : _textureCache | std::views::values)
        {
            const uint32_t id = val.GetId();
            glDeleteTextures(1, &id);
        }
        _textureCache.clear();
    }

    bool Texture::IsValid(const char* Path)
    {
        return _textureCache.contains(Path);
    }

    bool Texture::IsValid(const Texture Texture)
    {
        for (const auto& val : _textureCache | std::views::values)
        {
            if (val == Texture)
            {
                return true;
            }
        }
        return false;
    }

    std::string Texture::GetTexturePath(const Texture& Texture)
    {
        {
            for (const auto& [texture, path] : _textureCache)
            {
                if (path == Texture)
                {
                    return texture;
                }
            }

            return std::string();
        }
    }
}
