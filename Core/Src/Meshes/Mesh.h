#pragma once
#include <unordered_map>
#include <assimp/scene.h>

#include "Meshes/MeshFacesData.h"
#include "Meshes/MeshVerticesData.h"

namespace Core
{
    class Mesh final
    {
        friend class MeshManager;

    private:
        uint32_t _verticesIndex;
        uint32_t _verticesCount;
        uint32_t _facesIndex;
        uint32_t _facesCount;

    public:
        Mesh() : _verticesIndex(0), _verticesCount(0), _facesIndex(0), _facesCount(0)
        {
        }

        Mesh(const uint32_t VerticesIndex, const uint32_t VerticesCount, const uint32_t FacesIndex,
             const uint32_t FacesCount)
            : _verticesIndex(VerticesIndex),
              _verticesCount(VerticesCount),
              _facesIndex(FacesIndex),
              _facesCount(FacesCount)
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

        [[nodiscard]] uint32_t GetFacesIndex() const
        {
            return _facesIndex;
        }

        [[nodiscard]] uint32_t GetFacesCount() const
        {
            return _facesCount;
        }

        bool operator==(const Mesh& Mesh) const
        {
            return memcmp(this, &Mesh, sizeof(Mesh)) == 0;
        }
    };
}
