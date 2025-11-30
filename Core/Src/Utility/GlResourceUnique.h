#pragma once
#include <cstdint>
#include <utility>

#include "glad/glad.h"

namespace Core::Detail
{
    /**
     * @brief Traits struct for managing OpenGL Buffer objects using DSA.
     */
    struct BufferTraits
    {
        /**
         * @brief Creates a new OpenGL Buffer using Direct State Access (DSA).
         * @return The ID of the created buffer.
         */
        static uint32_t Create()
        {
            uint32_t id;
            glCreateBuffers(1, &id);
            return id;
        }

        /**
         * @brief Deletes an OpenGL Buffer.
         * @param id The ID of the buffer to delete.
         */
        static void Delete(const uint32_t id)
        {
            glDeleteBuffers(1, &id);
        }
    };

    /**
     * @brief Traits struct for managing OpenGL Texture objects using DSA.
     */
    struct TextureTraits
    {
        /**
         * @brief Creates a new OpenGL Texture using Direct State Access (DSA).
         * @param Target The texture target (e.g., GL_TEXTURE_2D).
         * @return The ID of the created texture.
         */
        static uint32_t Create(const GLenum Target)
        {
            uint32_t id;
            glCreateTextures(Target, 1, &id);
            return id;
        }

        /**
         * @brief Deletes an OpenGL Texture.
         * @param id The ID of the texture to delete.
         */
        static void Delete(const uint32_t id)
        {
            glDeleteTextures(1, &id);
        }
    };

    /**
     * @brief Traits struct for managing OpenGL Vertex Array Objects (VAO) using DSA.
     */
    struct VertexArrayTraits
    {
        /**
         * @brief Creates a new Vertex Array Object using Direct State Access (DSA).
         * @return The ID of the created VAO.
         */
        static uint32_t Create()
        {
            uint32_t id;
            glCreateVertexArrays(1, &id);
            return id;
        }

        /**
         * @brief Deletes a Vertex Array Object.
         * @param id The ID of the VAO to delete.
         */
        static void Delete(const uint32_t id)
        {
            glDeleteVertexArrays(1, &id);
        }
    };

    /**
     * @brief Traits struct for managing OpenGL Framebuffer Objects (FBO) using DSA.
     */
    struct FramebufferTraits
    {
        /**
         * @brief Creates a new Framebuffer Object using Direct State Access (DSA).
         * @return The ID of the created FBO.
         */
        static uint32_t Create()
        {
            uint32_t id;
            glCreateFramebuffers(1, &id);
            return id;
        }

        /**
         * @brief Deletes a Framebuffer Object.
         * @param id The ID of the FBO to delete.
         */
        static void Delete(const uint32_t id)
        {
            glDeleteFramebuffers(1, &id);
        }
    };

    /**
     * @brief Traits struct for managing OpenGL Renderbuffer Objects using DSA.
     */
    struct RenderbufferTraits
    {
        /**
         * @brief Creates a new Renderbuffer Object using Direct State Access (DSA).
         * @return The ID of the created Renderbuffer.
         */
        static uint32_t Create()
        {
            uint32_t id;
            glCreateRenderbuffers(1, &id);
            return id;
        }

        /**
         * @brief Deletes a Renderbuffer Object.
         * @param id The ID of the Renderbuffer to delete.
         */
        static void Delete(const uint32_t id)
        {
            glDeleteRenderbuffers(1, &id);
        }
    };

    /**
     * @brief Traits struct for managing OpenGL Sampler Objects using DSA.
     */
    struct SamplerTraits
    {
        /**
         * @brief Creates a new Sampler Object using Direct State Access (DSA).
         * @return The ID of the created Sampler.
         */
        static uint32_t Create()
        {
            uint32_t id;
            glCreateSamplers(1, &id);
            return id;
        }

        /**
         * @brief Deletes a Sampler Object.
         * @param id The ID of the Sampler to delete.
         */
        static void Delete(const uint32_t id)
        {
            glDeleteSamplers(1, &id);
        }
    };

    /**
     * @brief Traits struct for managing OpenGL Query Objects using DSA.
     */
    struct QueryTraits
    {
        /**
         * @brief Creates a new Query Object using Direct State Access (DSA).
         * @param target The query target (e.g., GL_TIME_ELAPSED).
         * @return The ID of the created Query.
         */
        static uint32_t Create(const GLenum target)
        {
            uint32_t id;
            glCreateQueries(target, 1, &id);
            return id;
        }

        /**
         * @brief Deletes a Query Object.
         * @param id The ID of the Query to delete.
         */
        static void Delete(const uint32_t id)
        {
            glDeleteQueries(1, &id);
        }
    };

    template<typename Traits>
    class GlResourceUnique
    {
    private:
        uint32_t _id;

    public:
        /**
         * @brief Factory method to create a new OpenGL resource.
         *
         * Forwards arguments to the underlying Traits::Create method.
         *
         * @tparam Args Argument types required by the specific resource creation (e.g., GLenum Target).
         * @param args Arguments forwarded to Traits::Create.
         * @return A new instance of GlResourceUnique managing the created resource.
         */
        template<typename... Args>
        [[nodiscard]] static GlResourceUnique Create(Args&&... args)
        {
            return GlResourceUnique(Traits::Create(std::forward<Args>(args)...));
        }

    public:
        GlResourceUnique(const GlResourceUnique&) = delete;

        GlResourceUnique& operator=(const GlResourceUnique&) = delete;

        /**
         * @brief Default constructor. Initializes the resource ID to 0 (invalid).
         */
        [[nodiscard]] GlResourceUnique() : _id(0)
        {
        }

        /**
         * @brief Move constructor. Transfers ownership of the resource from another instance.
         * @param Other The instance to move from. It will be reset to 0.
         */
        [[nodiscard]] GlResourceUnique(GlResourceUnique&& Other) noexcept
            : _id(std::exchange(Other._id, 0))
        {
        }

        /**
         * @brief Move assignment operator.
         *
         * Deletes the currently held resource (if any) and takes ownership of the resource from Other.
         *
         * @param Other The instance to move from. It will be reset to 0.
         * @return Reference to this instance.
         */
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

        /**
         * @brief Explicit constructor taking a raw OpenGL ID.
         *
         * This instance takes ownership of the provided ID.
         *
         * @param Id The raw OpenGL resource ID.
         */
        [[nodiscard]] explicit GlResourceUnique(const uint32_t Id) : _id(Id)
        {
        }

        /**
         * @brief Gets the raw OpenGL resource ID.
         * @return The underlying OpenGL ID.
         */
        [[nodiscard]] uint32_t Get() const
        {
            return _id;
        }

        /**
         * @brief Resets the managed resource.
         *
         * Deletes the currently managed resource and adopts a provided one.
         * Provide 0 to simply delete the current resource without adopting a new one.
         *
         * @param Id The new raw OpenGL ID to manage.
         */
        void Reset(const uint32_t Id)
        {
            if (_id == Id)
            {
                return;
            }

            Delete();
            _id = Id;
        }

        /**
         * @brief Releases ownership of the managed resource without deleting it.
         * @return The raw OpenGL ID that was being managed. The internal ID is set to 0.
         */
        uint32_t Release()
        {
            return std::exchange(_id, 0);
        }

        /**
         * @brief Destructor. Deletes the managed resource if valid.
         */
        ~GlResourceUnique()
        {
            Delete();
        }

        /**
         * @brief Swaps the contents of two GlResourceUnique instances.
         * @param First The first instance.
         * @param Second The second instance.
         */
        friend void Swap(GlResourceUnique& First, GlResourceUnique& Second) noexcept
        {
            std::swap(First._id, Second._id);
        }

        /**
         * @brief Checks if the resource is valid (ID != 0).
         * @return True if valid, false otherwise.
         */
        [[nodiscard]] constexpr operator bool() const
        {
            return _id != 0;
        }

        /**
         * @brief Implicit conversion to the raw OpenGL ID.
         *
         * Allows passing the object directly to OpenGL functions expecting an uint32_t (GLuint).
         *
         * @return The raw OpenGL ID.
         */
        [[nodiscard]] constexpr operator uint32_t() const
        {
            return _id;
        }

        /**
         * @brief Equality operator comparing two instances.
         */
        [[nodiscard]] friend bool operator==(const GlResourceUnique& First, const GlResourceUnique& Second)
        {
            return First._id == Second._id;
        }

        /**
         * @brief Inequality operator comparing two instances.
         */
        [[nodiscard]] friend bool operator!=(const GlResourceUnique& First, const GlResourceUnique& Second)
        {
            return First._id != Second._id;
        }

        /**
         * @brief Equality operator comparing wrapper with raw ID.
         */
        [[nodiscard]] friend bool operator==(const GlResourceUnique& First, const uint32_t Second)
        {
            return First._id == Second;
        }

        /**
         * @brief Inequality operator comparing wrapper with raw ID.
         */
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
    /**
     * @brief RAII wrapper for OpenGL Buffers (e.g., VBO, EBO, UBO, SSBO).
     * Automatically handles glCreateBuffers and glDeleteBuffers.
     */
    using GlBufferUnique = Detail::GlResourceUnique<Detail::BufferTraits>;
    /**
     * @brief RAII wrapper for OpenGL Textures.
     * Automatically handles glCreateTextures and glDeleteTextures.
     */
    using GlTextureUnique = Detail::GlResourceUnique<Detail::TextureTraits>;
    /**
     * @brief RAII wrapper for Vertex Array Objects (VAO).
     * Automatically handles glCreateVertexArrays and glDeleteVertexArrays.
     */
    using GlVertexArrayUnique = Detail::GlResourceUnique<Detail::VertexArrayTraits>;
    /**
     * @brief RAII wrapper for Framebuffer Objects (FBO).
     * Automatically handles glCreateFramebuffers and glDeleteFramebuffers.
     */
    using GlFramebufferUnique = Detail::GlResourceUnique<Detail::FramebufferTraits>;
    /**
     * @brief RAII wrapper for Renderbuffer Objects (RBO).
     * Automatically handles glCreateRenderbuffers and glDeleteRenderbuffers.
     */
    using GlRenderbufferUnique = Detail::GlResourceUnique<Detail::RenderbufferTraits>;
    /**
     * @brief RAII wrapper for OpenGL Sampler Objects.
     * Automatically handles glCreateSamplers and glDeleteSamplers.
     */
    using GlSamplerUnique = Detail::GlResourceUnique<Detail::SamplerTraits>;
    /**
     * @brief RAII wrapper for OpenGL Query Objects (e.g., Timer, Occlusion).
     * Automatically handles glCreateQueries and glDeleteQueries.
     */
    using GlQueryUnique = Detail::GlResourceUnique<Detail::QueryTraits>;
}
