#pragma once
#include "Meshes/Mesh.h"

namespace Core::MeshUtility
{
    [[nodiscard]] float CalculateMeshVolume(const Mesh& Mesh);
}
