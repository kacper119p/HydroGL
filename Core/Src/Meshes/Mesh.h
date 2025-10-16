#pragma once
#include <assimp/scene.h>

namespace Core
{
    class Mesh final
    {
        friend class MeshManager;

    private:
        uint32_t _verticesIndex;
        uint32_t _verticesCount;
        uint32_t _indicesIndex;
        uint32_t _indicesCount;

    public:
        Mesh() : _verticesIndex(0), _verticesCount(0), _indicesIndex(0), _indicesCount(0)
        {
        }

        Mesh(const uint32_t VerticesIndex, const uint32_t VerticesCount, const uint32_t FacesIndex,
             const uint32_t FacesCount) : _verticesIndex(VerticesIndex),
                                          _verticesCount(VerticesCount),
                                          _indicesIndex(FacesIndex),
                                          _indicesCount(FacesCount)
        {
        }

    public:
        ~Mesh() = default;

    public:
        [[nodiscard]] uint32_t GetVerticesIndex() const
        {
            return _verticesIndex;
        }

        [[nodiscard]] uint32_t GetVerticesCount() const
        {
            return _verticesCount;
        }

        [[nodiscard]] uint32_t GetIndicesIndex() const
        {
            return _indicesIndex;
        }

        [[nodiscard]] uint32_t GetIndicesCount() const
        {
            return _indicesCount;
        }

        [[nodiscard]] uint32_t GetTriangleCount() const
        {
            return _indicesCount / 3;
        }

        bool operator==(const Mesh& Mesh) const
        {
            return memcmp(this, &Mesh, sizeof(Mesh)) == 0;
        }

        bool operator!=(const Mesh& Mesh) const
        {
            return !(*this == Mesh);
        }
    };
}
