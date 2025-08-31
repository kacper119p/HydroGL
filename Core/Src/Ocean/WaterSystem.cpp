#include "WaterSystem.h"

#include <chrono>

#include "Rendering/Shaders/ShaderBuilder.h"

namespace Core
{
    WaterSystem::WaterSystem() : _oceanGrid(100.0f, 10, 10), _waterShader(
                                     ShaderBuilder().SetStage("Assets/Shaders/Water/Water.vert",
                                                              ShaderStage::VertexShader)
                                     .SetStage("Assets/Shaders/Water/Water.tesc",
                                               ShaderStage::TesselationControlShader)
                                     .SetStage("Assets/Shaders/Water/Water.tese",
                                               ShaderStage::TesselationEvaluationShader)
                                     .SetStage("Assets/Shaders/Water/Water.frag",
                                               ShaderStage::FragmentShader)
                                     .Build()),
                                 _viewMatrixLocation(_waterShader.GetUniformLocation("ViewMatrix")),
                                 _projectionMatrixLocation(_waterShader.GetUniformLocation("ProjectionMatrix")),
                                 _cameraPositionLocation(_waterShader.GetUniformLocation("CameraPosition"))
    {
    }

    void WaterSystem::Render(const TransformComponent& CameraTransform, const CameraComponent& Camera) const
    {
        _waterShader.Use();
        Shader::SetUniform(_viewMatrixLocation, glm::inverse(CameraTransform.GetLocalToWorldMatrix()));
        Shader::SetUniform(_projectionMatrixLocation, Camera.ProjectionMatrix);
        Shader::SetUniform(_cameraPositionLocation, CameraTransform.GetWorldPosition());

        _waterShader.SetUniform("Time",
                                std::chrono::duration<float>(
                                    std::chrono::high_resolution_clock::now().time_since_epoch()).count());

        _oceanGrid.Draw();
    }
}
