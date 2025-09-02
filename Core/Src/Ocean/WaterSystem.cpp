#include "WaterSystem.h"

#include <chrono>

#include "Rendering/Shaders/ShaderBuilder.h"

namespace Core
{
    WaterSystem::WaterSystem() : _oceanGrid(1000.0f, 100, 10), _waterShader(
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
                                 _cameraPositionLocation(_waterShader.GetUniformLocation("CameraPosition")),
                                 _timeLocation(_waterShader.GetUniformLocation("Time"))
    {
    }

    void WaterSystem::Render(const TransformComponent& CameraTransform, const CameraComponent& Camera) const
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        const float currentTime
                = std::chrono::duration<float>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

        _waterShader.Use();
        Shader::SetUniform(_viewMatrixLocation, glm::inverse(CameraTransform.GetLocalToWorldMatrix()));
        Shader::SetUniform(_projectionMatrixLocation, Camera.ProjectionMatrix);
        Shader::SetUniform(_cameraPositionLocation, CameraTransform.GetWorldPosition());
        Shader::SetUniform(_timeLocation, currentTime);

        _oceanGrid.Draw();

        //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDisable(GL_BLEND);
    }
}
