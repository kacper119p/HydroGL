#pragma once
#include <cstdint>
#include <cstdlib>

namespace Core
{
    struct MeshFacesData
    {
    public:
        uint32_t* Indices;

    public:
        explicit MeshFacesData(const uint32_t Capacity)
        {
            Indices = reinterpret_cast<uint32_t*>(std::malloc(Capacity * sizeof(uint32_t)));
        }

        ~MeshFacesData()
        {
            std::free(Indices);
        }
    };
}
