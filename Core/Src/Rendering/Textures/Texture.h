#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>

#include "glad/glad.h"

namespace Core
{
    /**
     * @brief Represents a texture resource.
     * Manages the lifecycle, binding, and access to OpenGL texture objects loaded from files.
     * Includes static utilities for resource management and caching.
     */
    class Texture
    {
    private:
        static std::unordered_map<std::string, Texture> _textureCache;

    private:
        uint32_t _textureId;

    public:
        /**
         * @brief Constructs an invalid texture object.
         */
        Texture() : _textureId(0)
        {
        }

        /**
         * @brief Wraps an existing texture identifier.
         * @param TextureId The texture identifier to wrap.
         */
        explicit Texture(const uint32_t TextureId) : _textureId(TextureId)
        {
        }

    public:
        /**
         * @brief Gets the underlying texture identifier.
         * @return The texture ID.
         */
        [[nodiscard]] uint32_t GetId() const
        {
            return _textureId;
        }

        /**
         * @brief Gets a GPU handle for bindless access.
         * @return A 64-bit handle representing the texture, or 0 if invalid.
         */
        [[nodiscard]] uint64_t GetHandleReadonly() const
        {
            if (_textureId == 0)
            {
                return 0;
            }
            const uint64_t handle = glGetTextureHandleARB(_textureId);
            return handle;
        }

        /**
         * @brief Binds this texture to a specific texture unit.
         * @param TextureUnit The index of the texture unit.
         */
        void BindTextureUnit(const uint32_t TextureUnit) const
        {
            glBindTextureUnit(TextureUnit, _textureId);
        }

    public:
        /**
         * @brief Loads a texture from a file if not already loaded, otherwise retrieves an id of an already loaded
         * texture.
         * @param Path Path to a texture file.
         * @return A loaded texture.
         */
        static Texture GetTexture(const std::string_view& Path);

        /**
         * @brief Frees resources used by an existing texture.
         * @param Path Path to a texture file.
         * @return True if the texture existed prior to a method call. False otherwise.
         */
        static bool DeleteTexture(const std::string_view& Path);

        /**
         * @brief Frees resources used by an existing texture.
         * @param Texture Texture to delete
         * @return True if a texture existed prior to a method call. False otherwise.
         */
        static bool DeleteTexture(const Texture& Texture);

        /**
         * @brief Frees resources used by all textures.
         */
        static void DeleteAllTextures();

        /**
         * @brief Checks if a texture is loaded and ready to use.
         * @param Path Path to a texture file.
         * @return True if a texture is loaded. False otherwise.
         */
        [[nodiscard]] static bool IsValid(const char* Path);

        /**
         * @brief Checks if a texture is loaded and ready to use.
         * @param Texture Texture to be checked.
         * @return True if a texture is loaded. False otherwise.
         */
        [[nodiscard]] static bool IsValid(Texture Texture);

        /**
         * @brief Checks if a texture is loaded and ready to use.
         * @param Texture Texture to be found.
         * @return
         */
        [[nodiscard]] static std::string GetTexturePath(const Texture& Texture);

    public:
        bool operator==(const Texture Other) const
        {
            return _textureId == Other._textureId;
        }

        bool operator!=(const Texture Other) const
        {
            return !(*this == Other);
        }
    };
}
