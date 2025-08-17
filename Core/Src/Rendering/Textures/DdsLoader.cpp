#include "DDSLoader.h"

#include <glad/glad.h>
#include <glm/ext/scalar_int_sized.hpp>

#include "spdlog/spdlog.h"

#define NOTIFY_FAILED spdlog::error("Loading texture from {0} failed.", FilePath);

namespace Core
{
    uint32_t DdsLoader::LoadDds(const std::string_view& FilePath, uint32_t& Width, uint32_t& Height)
    {
        uint32_t textureId = 0;

        FILE* file;
        if (fopen_s(&file, FilePath.data(), "rb") != 0)
        {
#if DEBUG
            NOTIFY_FAILED
#endif
            return textureId;
        }

        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        int32_t fileCode;
        fread(&fileCode, 1, 4, file);
#if DEBUG
        if (fileCode != DDS_MAGIC)
        {
            fclose(file);
            return 0;
        }
#endif

        DDS_HEADER header{};
        DDS_HEADER_DXT10 headerDxt{};

        fread(&header, sizeof(DDS_HEADER), 1, file);
        fread(&headerDxt, sizeof(DDS_HEADER_DXT10), 1, file);

        Height = header.dwHeight;
        Width = header.dwWidth;

#if DEBUG
        constexpr char dx10FourCc[4] = {'D', 'X', '1', '0'};
        if (memcmp(&header.ddspf.dwFourCC, dx10FourCc, sizeof(DWORD)) != 0)
        {
            fclose(file);
            return 0;
        }
#endif
        GlTextureDescriptor textureDescriptor{};
        DdsToGlFormat(headerDxt.dxgiFormat, textureDescriptor);

        if (textureDescriptor.InternalFormat == GL_NONE)
        {
#if DEBUG
            NOTIFY_FAILED
#endif
            fclose(file);
            return 0;
        }
        void* buffer = malloc(file_size - sizeof(DDS_HEADER) - sizeof(DDS_HEADER_DXT10));
        if (buffer == nullptr)
        {
#if DEBUG
            NOTIFY_FAILED
#endif
            free(buffer);
            fclose(file);
            return 0;
        }

        fread(buffer, 1, file_size, file);

        if ((header.dwCaps2 & DDS_CUBEMAP) != 0)
        {
            textureId = CreateTextureCube(textureDescriptor, header, buffer);
        } else
        {
            textureId = CreateTexture2D(textureDescriptor, header, buffer);
        }

        free(buffer);
        fclose(file);

        return textureId;
    }

    void DdsLoader::DdsToGlFormat(const DXGI_FORMAT DxgiFormat, GlTextureDescriptor& TextureDescriptor)
    {
        switch (DxgiFormat)
        {
            case DXGI_FORMAT_BC1_UNORM: // DXT1
                TextureDescriptor.InternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
                TextureDescriptor.BlockSize = 8;
                TextureDescriptor.DataType = GL_NONE;
                TextureDescriptor.Format = GL_NONE;
                return;
            case DXGI_FORMAT_BC2_UNORM: // DXT3
                TextureDescriptor.InternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
                TextureDescriptor.BlockSize = 16;
                TextureDescriptor.DataType = GL_NONE;
                TextureDescriptor.Format = GL_NONE;
                return;
            case DXGI_FORMAT_BC3_UNORM: // DXT5
                TextureDescriptor.InternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
                TextureDescriptor.BlockSize = 16;
                TextureDescriptor.DataType = GL_NONE;
                TextureDescriptor.Format = GL_NONE;
                return;
            case DXGI_FORMAT_BC7_UNORM: // BPTC
                TextureDescriptor.InternalFormat = GL_COMPRESSED_RGBA_BPTC_UNORM;
                TextureDescriptor.BlockSize = 16;
                TextureDescriptor.DataType = GL_NONE;
                TextureDescriptor.Format = GL_NONE;
                return;
            case DXGI_FORMAT_BC6H_UF16: // HDR compressed (unsigned float)
                TextureDescriptor.InternalFormat = GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT;
                TextureDescriptor.BlockSize = 16;
                TextureDescriptor.DataType = GL_NONE;
                TextureDescriptor.Format = GL_NONE;
                return;
            case DXGI_FORMAT_BC6H_SF16: // HDR compressed (signed float)
                TextureDescriptor.InternalFormat = GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT;
                TextureDescriptor.BlockSize = 16;
                TextureDescriptor.DataType = GL_NONE;
                TextureDescriptor.Format = GL_NONE;
                return;
            case DXGI_FORMAT_R16G16B16A16_UNORM: // uncompressed 16 bit
                TextureDescriptor.InternalFormat = GL_RGBA16;
                TextureDescriptor.BlockSize = 8;
                TextureDescriptor.DataType = GL_UNSIGNED_SHORT;
                TextureDescriptor.Format = GL_RGBA;
                return;
            case DXGI_FORMAT_R8G8B8A8_UNORM: // uncompressed 8 bit
                TextureDescriptor.InternalFormat = GL_RGBA8;
                TextureDescriptor.BlockSize = 4;
                TextureDescriptor.DataType = GL_UNSIGNED_BYTE;
                TextureDescriptor.Format = GL_RGBA;
                return;
            case DXGI_FORMAT_R8_UNORM: // uncompressed 8 bit red
                TextureDescriptor.InternalFormat = GL_R8;
                TextureDescriptor.BlockSize = 1;
                TextureDescriptor.DataType = GL_UNSIGNED_BYTE;
                TextureDescriptor.Format = GL_RED;
                return;
            case DXGI_FORMAT_R16_UNORM: // uncompressed 16 bit red
                TextureDescriptor.InternalFormat = GL_RGBA8;
                TextureDescriptor.BlockSize = 4;
                TextureDescriptor.DataType = GL_UNSIGNED_SHORT;
                TextureDescriptor.Format = GL_RED;
                return;
            case DXGI_FORMAT_R16G16B16A16_FLOAT: // uncompressed 16 bit HDR
                TextureDescriptor.InternalFormat = GL_RGBA16F;
                TextureDescriptor.BlockSize = 8;
                TextureDescriptor.DataType = GL_HALF_FLOAT;
                TextureDescriptor.Format = GL_RGBA;
                return;
            default: // Unsupported format
                TextureDescriptor.InternalFormat = GL_NONE;
                TextureDescriptor.BlockSize = 0;
                TextureDescriptor.DataType = GL_NONE;
                TextureDescriptor.Format = GL_NONE;
        }
    }

    uint32_t DdsLoader::CreateTexture2D(const GlTextureDescriptor& TextureDescriptor, const DDS_HEADER& DdsHeader,
                                        const void* const Buffer)
    {
        uint32_t textureId = 0;
        glGenTextures(1, &textureId);
        if (textureId == 0)
        {
            return 0;
        }

        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(DdsHeader.dwMipMapCount - 1));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, 16.0f);

        uint32_t offset = 0;
        int32_t size = 0;

        uint32_t mipMapCount = DdsHeader.dwMipMapCount;

        int32_t h = static_cast<int32_t>(DdsHeader.dwHeight);
        int32_t w = static_cast<int32_t>(DdsHeader.dwWidth);

        // loop through sending block at a time with the magic formula
        // upload to opengl properly, note the offset transverses the pointer
        // assumes each mipmap is 1/2 the size of the previous mipmap
        if (TextureDescriptor.Format != GL_NONE)
        {
            for (uint32_t i = 0; i < mipMapCount; i++)
            {
                if (w == 0 || h == 0)
                {
                    // discard any odd mipmaps 0x1 0x2 resolutions
                    mipMapCount--;
                    continue;
                }
                size = w * h * TextureDescriptor.BlockSize;
                glTexImage2D(GL_TEXTURE_2D, static_cast<GLint>(i), TextureDescriptor.InternalFormat, w, h, 0,
                             TextureDescriptor.Format, TextureDescriptor.DataType,
                             reinterpret_cast<const uint8_t*>(Buffer) + offset);
                offset += size;
                w /= 2;
                h /= 2;
            }
        } else
        {
            for (unsigned int i = 0; i < mipMapCount; i++)
            {
                if (w == 0 || h == 0)
                {
                    // discard any odd mipmaps 0x1 0x2 resolutions
                    mipMapCount--;
                    continue;
                }
                size = ((w + 3) / 4) * ((h + 3) / 4) * TextureDescriptor.BlockSize;
                glCompressedTexImage2D(GL_TEXTURE_2D, static_cast<int32_t>(i), TextureDescriptor.InternalFormat, w, h,
                                       0, size, reinterpret_cast<const uint8_t*>(Buffer) + offset);
                offset += size;
                w /= 2;
                h /= 2;
            }
        }

        // discard any odd mipmaps, ensure a complete texture
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, static_cast<int32_t>(mipMapCount - 1));

        return textureId;
    }

    uint32_t DdsLoader::CreateTextureCube(const GlTextureDescriptor& TextureDescriptor, const DDS_HEADER& DdsHeader,
                                          const void* const Buffer)
    {
        uint32_t textureId = 0;
        glGenTextures(1, &textureId);
        if (textureId == 0)
        {
            return 0;
        }

        glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, static_cast<int32_t>(DdsHeader.dwMipMapCount - 1));
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY, 16.0f);

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


        const uint32_t mipMapCount = DdsHeader.dwMipMapCount;

        constexpr uint8_t faceCount = 6;
        constexpr uint32_t faces[faceCount]{
            DDS_CUBEMAP_POSITIVEX, DDS_CUBEMAP_NEGATIVEX, DDS_CUBEMAP_POSITIVEY,
            DDS_CUBEMAP_NEGATIVEY, DDS_CUBEMAP_POSITIVEZ, DDS_CUBEMAP_NEGATIVEZ
        };

        uint32_t offset = 0;
        for (uint32_t face = 0; face < faceCount; face++)
        {
            if ((DdsHeader.dwCaps2 & faces[face]) == 0)
            {
                continue;
            }

            uint32_t h = DdsHeader.dwHeight;
            uint32_t w = DdsHeader.dwWidth;

            for (uint32_t mip = 0; mip < mipMapCount; mip++)
            {
                if (TextureDescriptor.Format != GL_NONE) // Uncompressed
                {
                    const uint32_t size = w * h * TextureDescriptor.BlockSize;
                    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, static_cast<int32_t>(mip),
                                 TextureDescriptor.InternalFormat, static_cast<int32_t>(w), static_cast<int32_t>(h), 0,
                                 TextureDescriptor.Format, TextureDescriptor.DataType,
                                 reinterpret_cast<const uint8_t*>(Buffer) + offset);
                    offset += size;
                } else
                {
                    const uint32_t size = ((w + 3) / 4) * ((h + 3) / 4) * TextureDescriptor.BlockSize;
                    glCompressedTexImage2D(
                        GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, static_cast<int32_t>(mip),
                        TextureDescriptor.InternalFormat,
                        static_cast<int32_t>(w), static_cast<int32_t>(h), 0, static_cast<int32_t>(size),
                        reinterpret_cast<const uint8_t*>(Buffer) + offset);
                    offset += size;
                }
                w = std::max(1u, w / 2u);
                h = std::max(1u, h / 2u);
            }
        }

        return textureId;
    }
}
