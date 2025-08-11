#pragma once
#include <glm/glm.hpp>

namespace Core
{
    struct MeshVerticesData
    {
    public:
        static constexpr size_t VertexSize
                = sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(glm::vec3) + sizeof(glm::vec3);

    public:
        glm::vec3* Positions;
        glm::vec2* TexCoords;
        glm::vec3* Normals;
        glm::vec3* Tangents;

    public:
        explicit MeshVerticesData(const uint32_t Capacity)
        {
            void* memory = malloc(
                Capacity * VertexSize);
            Positions = reinterpret_cast<glm::vec3*>(memory);
            TexCoords = reinterpret_cast<glm::vec2*>(Positions + Capacity);
            Normals = reinterpret_cast<glm::vec3*>(TexCoords + Capacity);
            Tangents = reinterpret_cast<glm::vec3*>(Normals + Capacity);
        }

        ~MeshVerticesData()
        {
            free(Positions);
        }
    };
}
