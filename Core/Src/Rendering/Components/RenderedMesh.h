#pragma once
#include "Meshes/Mesh.h"
#include "Rendering/Materials/Material.h"

namespace Core
{
    class RenderedMesh
    {
    public:
        Mesh Mesh;
        MaterialId Material;
    };
}
