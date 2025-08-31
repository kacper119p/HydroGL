#pragma once
#include "Camera/CameraComponent.h"
#include "Ocean/OceanGrid.h"
#include "Rendering/Shaders/Shader.h"
#include "Scene/TransformComponent.h"

namespace Core
{
    class WaterSystem
    {
    private:
        OceanGrid _oceanGrid;

        Shader _waterShader;
        int32_t _viewMatrixLocation;
        int32_t _projectionMatrixLocation;
        int32_t _cameraPositionLocation;

    public:
        WaterSystem();

    public:
        void Render(const TransformComponent& CameraTransform, const CameraComponent& Camera) const;
    };
}
