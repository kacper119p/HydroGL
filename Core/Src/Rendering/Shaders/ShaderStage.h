#pragma once

namespace Core
{
    enum class ShaderStage : uint8_t
    {
        VertexShader = 0,
        TesselationControlShader = 1,
        TesselationEvaluationShader = 2,
        GeometryShader = 3,
        FragmentShader = 4,
        ComputeShader = 5,
        None = 255
    };
} // namespace Core
