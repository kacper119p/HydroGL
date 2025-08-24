#pragma once
#include "Meshes/Mesh.h"
#include "Rendering/Materials/Material.h"

namespace Core
{
    struct RenderedMeshComponent
    {
    public:
        Mesh Mesh;
        MaterialId Material;
    };
}
