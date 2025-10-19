#pragma once
#include "Meshes/Mesh.h"

namespace Core
{
    class WaterEffectorComponent
    {
    private:
        Mesh _mesh;

    public:
        explicit WaterEffectorComponent(const Mesh& Mesh)
            : _mesh(Mesh)
        {
        }

        [[nodiscard]] Core::Mesh GetMesh() const
        {
            return _mesh;
        }

        void SetMesh(const Core::Mesh& Mesh)
        {
            this->_mesh = Mesh;
        }
    };
}
