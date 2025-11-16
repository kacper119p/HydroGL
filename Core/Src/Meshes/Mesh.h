#pragma once
#include <assimp/scene.h>

#include "Meshes/BoundingBox.h"

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
        BoundingBox _boundingBox;

    public:
        Mesh() : _verticesIndex(0), _verticesCount(0), _indicesIndex(0), _indicesCount(0)
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

        [[nodiscard]] BoundingBox GetBoundingBox() const
        {
            return _boundingBox;
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
