#pragma once
#include <cstdint>

#include "glad/glad.h"

namespace Core
{
    class Texture
    {
    private:
        uint32_t _textureId;

    public:
        Texture() : _textureId(0) {}

        explicit Texture(const uint32_t TextureId) : _textureId(TextureId) {}

    public:
        [[nodiscard]] uint32_t GetId() const { return _textureId; }

        [[nodiscard]] uint64_t GetHandleReadonly() const
        {
            if (_textureId == 0)
            {
                return 0;
            }
            const uint64_t handle = glGetTextureHandleARB(_textureId);
            return handle;
        }

        bool operator==(const Texture Other) const { return _textureId == Other._textureId; }

        bool operator!=(const Texture Other) const { return !(*this == Other); }
    };
} // namespace Core
