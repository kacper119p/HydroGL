#include "ShaderBuilder.h"

#include <Debug.h>
#include <spdlog/spdlog.h>

#include "GlslPreprocessor.h"

namespace Core
{

    static constexpr int32_t ErrorLogSize = 512;

    std::unordered_map<std::string, uint32_t> ShaderBuilder::ShaderStages;

    Shader ShaderBuilder::Build() const
    {
#if DEBUG
        if (_shaderStages[static_cast<uint8_t>(ShaderStage::ComputeShader)].Stage != ShaderStage::None)
        {
            for (uint8_t i = 0; i < static_cast<uint8_t>(ShaderStage::ComputeShader); ++i)
            {
                CHECK_MESSAGE(_shaderStages[i].Stage == ShaderStage::None, "Incompatible shader stages declared");
            }
        }
#endif

        Shader shader;
        if (_shaderStages[static_cast<uint8_t>(ShaderStage::ComputeShader)].Stage != ShaderStage::None)
        {
            shader = CreateComputeShader();
        }
        else
        {
            shader = CreateShader();
        }

        // TODO: Error handling.

        return shader;
    }

    uint32_t ShaderBuilder::CompileShaderStage(const std::string& FilePath, const GLenum Type)
    {
        const std::string preprocessedSource = GLSLPreprocessor::PreprocessFile(FilePath);
        const GLchar* const source = preprocessedSource.c_str();

        const uint32_t shaderId = glCreateShader(Type);
        glShaderSource(shaderId, 1, &source, nullptr);
        glCompileShader(shaderId);

        int32_t success;
        glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            char errorInfo[ErrorLogSize];
            glGetShaderInfoLog(shaderId, ErrorLogSize, nullptr, errorInfo);
            spdlog::error("Failed to compile shader: ", FilePath, ": ", errorInfo);
            glDeleteShader(shaderId);
            return 0;
        }
        return shaderId;
    }

    uint32_t ShaderBuilder::GetShaderStage(const std::string& FilePath, const GLenum Type)
    {
        if (const auto iterator = ShaderStages.find(FilePath); iterator != ShaderStages.end())
        {
            return iterator->second;
        }

        uint32_t shaderId = CompileShaderStage(FilePath, Type);
        ShaderStages.emplace(FilePath, shaderId);
        return shaderId;
    }

    Shader ShaderBuilder::CreateShader() const
    {
        uint8_t stageCount = 0;
        uint32_t shaderPrograms[RenderShaderStages];

        for (uint8_t i = 0; i < RenderShaderStages; ++i)
        {
            if (_shaderStages[i].Stage == ShaderStage::None)
            {
                continue;
            }

            const uint32_t newShaderStage =
                    GetShaderStage(_shaderStages[i].Path, ShaderStageToGL(_shaderStages[i].Stage));

            if (newShaderStage == 0)
            {
                return Shader(0);
            }

            shaderPrograms[i] = newShaderStage;
            stageCount++;
        }

        const uint32_t programId = glCreateProgram();

        for (uint8_t i = 0; i < stageCount; ++i)
        {
            glAttachShader(programId, shaderPrograms[i]);
        }

        glLinkProgram(programId);
        for (uint8_t i = 0; i < RenderShaderStages; ++i)
        {
            glDetachShader(programId, shaderPrograms[i]);
        }

        int32_t success;
        glGetProgramiv(programId, GL_LINK_STATUS, &success);
        if (!success)
        {
            char errorInfo[ErrorLogSize];
            glGetProgramInfoLog(programId, ErrorLogSize, nullptr, errorInfo);
            glDeleteProgram(programId);
            spdlog::error("Failed to link shader: ", errorInfo);
        }

        return Shader(programId);
    }

    Shader ShaderBuilder::CreateComputeShader() const
    {

        const uint32_t ComputeShaderId =
                GetShaderStage(_shaderStages[static_cast<uint8_t>(ShaderStage::ComputeShader)].Path, GL_COMPUTE_SHADER);
        if (ComputeShaderId == 0)
        {
            return Shader(0);
        }

        const uint32_t programId = glCreateProgram();
        glAttachShader(programId, ComputeShaderId);
        glLinkProgram(programId);

        glDetachShader(programId, ComputeShaderId);

        int32_t success;
        glGetProgramiv(programId, GL_LINK_STATUS, &success);
        if (!success)
        {
            char errorInfo[ErrorLogSize];
            glGetProgramInfoLog(programId, ErrorLogSize, nullptr, errorInfo);
            glDeleteProgram(programId);
            spdlog::error("Failed to link shader: ", errorInfo);
        }

        return Shader(programId);
    }
    GLenum ShaderBuilder::ShaderStageToGL(ShaderStage Stage)
    {
        switch (Stage)
        {
            case ShaderStage::VertexShader:
                return GL_VERTEX_SHADER;
            case ShaderStage::TesselationControlShader:
                return GL_TESS_CONTROL_SHADER;
            case ShaderStage::TesselationEvaluationShader:
                return GL_TESS_EVALUATION_SHADER;
            case ShaderStage::GeometryShader:
                return GL_GEOMETRY_SHADER;
            case ShaderStage::FragmentShader:
                return GL_FRAGMENT_SHADER;
            case ShaderStage::ComputeShader:
                return GL_COMPUTE_SHADER;
            default:
                return GL_NONE;
        }
    }
} // namespace Core
