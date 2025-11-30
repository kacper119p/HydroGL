#pragma once
#include <cstdint>
#include <utility>

#include "glad/glad.h"

namespace Core::Detail
{
    struct BufferTraits
    {
        static uint32_t Create()
        {
            uint32_t id;
            glGenBuffers(1, &id);
            return id;
        }

        static void Delete(const uint32_t id)
        {
            glDeleteBuffers(1, &id);
        }
    };

    struct TextureTraits
    {
        static uint32_t Create()
        {
            uint32_t id;
            glGenTextures(1, &id);
            return id;
        }

        static void Delete(const uint32_t id)
        {
            glDeleteTextures(1, &id);
        }
    };

    struct VertexArrayTraits
    {
        static uint32_t Create()
        {
            uint32_t id;
            glGenVertexArrays(1, &id);
            return id;
        }

        static void Delete(const uint32_t id)
        {
            glDeleteVertexArrays(1, &id);
        }
    };

    struct FramebufferTraits
    {
        static uint32_t Create()
        {
            uint32_t id;
            glGenFramebuffers(1, &id);
            return id;
        }

        static void Delete(const uint32_t id)
        {
            glDeleteFramebuffers(1, &id);
        }
    };

    template<typename Traits>
    class GlResourceUnique
    {
    private:
        uint32_t _id;

    public:
        [[nodiscard]] static GlResourceUnique Create()
        {
            return GlResourceUnique(Traits::Create());
        }

    public:
        GlResourceUnique(const GlResourceUnique&) = delete;

        GlResourceUnique& operator=(const GlResourceUnique&) = delete;

        [[nodiscard]] GlResourceUnique() : _id(0)
        {
        }

        [[nodiscard]] GlResourceUnique(GlResourceUnique&& Other) noexcept
            : _id(std::exchange(Other._id, 0))
        {
        }

        GlResourceUnique& operator=(GlResourceUnique&& Other) noexcept
        {
            if (_id == Other._id)
            {
                return *this;
            }

            Delete();
            _id = std::exchange(Other._id, 0);
            return *this;
        }

        [[nodiscard]] explicit GlResourceUnique(const uint32_t Id) : _id(Id)
        {
        }

        [[nodiscard]] uint32_t Get() const
        {
            return _id;
        }

        void Reset(const uint32_t Id)
        {
            if (_id == Id)
            {
                return;
            }

            Delete();
            _id = Id;
        }

        uint32_t Release()
        {
            return std::exchange(_id, 0);
        }

        ~GlResourceUnique()
        {
            Delete();
        }

        friend void Swap(GlResourceUnique& First, GlResourceUnique& Second) noexcept
        {
            std::swap(First._id, Second._id);
        }

        [[nodiscard]] constexpr operator bool() const
        {
            return _id != 0;
        }

        [[nodiscard]] constexpr operator uint32_t() const
        {
            return _id;
        }

        [[nodiscard]] friend bool operator==(const GlResourceUnique& First, const GlResourceUnique& Second)
        {
            return First._id == Second._id;
        }

        [[nodiscard]] friend bool operator!=(const GlResourceUnique& First, const GlResourceUnique& Second)
        {
            return First._id != Second._id;
        }

        [[nodiscard]] friend bool operator==(const GlResourceUnique& First, const uint32_t Second)
        {
            return First._id == Second;
        }

        [[nodiscard]] friend bool operator!=(const GlResourceUnique& First, const uint32_t Second)
        {
            return First._id != Second;
        }

    private:
        void Delete()
        {
            if (_id != 0)
            {
                Traits::Delete(_id);
                _id = 0;
            }
        }
    };
}

namespace Core
{
    using glBufferUnique = Detail::GlResourceUnique<Detail::BufferTraits>;
    using glTextureUnique = Detail::GlResourceUnique<Detail::TextureTraits>;
    using glVertexArrayUnique = Detail::GlResourceUnique<Detail::VertexArrayTraits>;
    using glFramebufferUnique = Detail::GlResourceUnique<Detail::FramebufferTraits>;
}
