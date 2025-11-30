#pragma once

#include <cstdint>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Core
{
    class Shader final
    {
    private:
        uint32_t _id;

    public:
        Shader() : _id(0)
        {
        }

        explicit Shader(const uint32_t ProgramId) : _id(ProgramId)
        {
        }

        Shader(const Shader& Other) noexcept = default;

    public:
        [[nodiscard]] glm::ivec3 GetComputeShaderWorkGroupSize()
        {
            int localWorkGroupSize[3];
            glGetProgramiv(_id, GL_COMPUTE_WORK_GROUP_SIZE, localWorkGroupSize);
            return glm::ivec3(localWorkGroupSize[0], localWorkGroupSize[1], localWorkGroupSize[2]);
        }

        static void DispatchCompute(const glm::ivec3& WorkGroupCount)
        {
            glDispatchCompute(WorkGroupCount.x, WorkGroupCount.y, WorkGroupCount.z);
        }

        static void DispatchCompute(const int32_t WorkGroupCountX, const int32_t WorkGroupCountY,
                                    const int32_t WorkGroupCountZ)
        {
            glDispatchCompute(WorkGroupCountX, WorkGroupCountY, WorkGroupCountZ);
        }

        static void DispatchCompute()
        {
            glDispatchCompute(1, 1, 1);
        }

    public:
        void Use() const
        {
            glUseProgram(_id);
        }

        [[nodiscard]] bool IsNull() const
        {
            return _id == 0;
        }

        [[nodiscard]] unsigned int GetId() const
        {
            return _id;
        }

        [[nodiscard]] glm::ivec3 GetLocalWorkGroupSize() const
        {
            int32_t localWorkGroupSize[3];
            glGetProgramiv(_id, GL_COMPUTE_WORK_GROUP_SIZE, localWorkGroupSize);
            return glm::ivec3(localWorkGroupSize[0], localWorkGroupSize[1], localWorkGroupSize[2]);
        }

        [[nodiscard]] int32_t GetUniformLocation(const char* const Name) const
        {
            return glGetUniformLocation(_id, Name);
        }

        void SetUniform(const int32_t UniformId, const float Value) const
        {
            glProgramUniform1f(_id, UniformId, Value);
        }

        void SetUniform(const char* const Name, const float Value) const
        {
            SetUniform(GetUniformLocation(Name), Value);
        }

        void SetUniform(const int32_t UniformId, const glm::vec2& Value) const
        {
            glProgramUniform2f(_id, UniformId, Value.x, Value.y);
        }

        void SetUniform(const char* const Name, const glm::vec2& Value) const
        {
            SetUniform(GetUniformLocation(Name), Value);
        }

        void SetUniform(const int32_t UniformId, const glm::vec3& Value) const
        {
            glProgramUniform3f(_id, UniformId, Value.x, Value.y, Value.z);
        }

        void SetUniform(const char* const Name, const glm::vec3& Value) const
        {
            SetUniform(GetUniformLocation(Name), Value);
        }

        void SetUniform(const int32_t UniformId, const glm::vec4& Value) const
        {
            glProgramUniform4f(_id, UniformId, Value.x, Value.y, Value.z, Value.w);
        }

        void SetUniform(const char* const Name, const glm::vec4& Value) const
        {
            SetUniform(GetUniformLocation(Name), Value);
        }

        void SetUniform(const int32_t UniformId, const int32_t Value) const
        {
            glProgramUniform1i(_id, UniformId, Value);
        }

        void SetUniform(const char* const Name, const int32_t Value) const
        {
            SetUniform(GetUniformLocation(Name), Value);
        }

        void SetUniform(const int32_t UniformId, const glm::ivec2& Value) const
        {
            glProgramUniform2i(_id, UniformId, Value.x, Value.y);
        }

        void SetUniform(const char* const Name, const glm::ivec2& Value) const
        {
            SetUniform(GetUniformLocation(Name), Value);
        }

        void SetUniform(const int32_t UniformId, const glm::ivec3& Value) const
        {
            glProgramUniform3i(_id, UniformId, Value.x, Value.y, Value.z);
        }

        void SetUniform(const char* const Name, const glm::ivec3& Value) const
        {
            SetUniform(GetUniformLocation(Name), Value);
        }

        void SetUniform(const int32_t UniformId, const glm::ivec4& Value) const
        {
            glProgramUniform4i(_id, UniformId, Value.x, Value.y, Value.z, Value.w);
        }

        void SetUniform(const char* const Name, const glm::ivec4& Value) const
        {
            SetUniform(GetUniformLocation(Name), Value);
        }

        void SetUniform(const int32_t UniformId, const bool Value) const
        {
            glProgramUniform1i(_id, UniformId, static_cast<int>(Value));
        }

        void SetUniform(const char* const Name, const bool Value) const
        {
            SetUniform(GetUniformLocation(Name), Value);
        }

        void SetUniform(const int32_t UniformId, const uint32_t Value) const
        {
            glProgramUniform1ui(_id, UniformId, Value);
        }

        void SetUniform(const char* const Name, const uint32_t Value) const
        {
            SetUniform(GetUniformLocation(Name), Value);
        }

        void SetUniform(const int32_t UniformId, const glm::uvec2 Value) const
        {
            glProgramUniform2ui(_id, UniformId, Value.x, Value.y);
        }

        void SetUniform(const char* const Name, const glm::uvec2& Value) const
        {
            SetUniform(GetUniformLocation(Name), Value);
        }

        void SetUniform(const int32_t UniformId, const glm::uvec3& Value) const
        {
            glProgramUniform3ui(_id, UniformId, Value.x, Value.y, Value.z);
        }

        void SetUniform(const char* const Name, const glm::uvec3& Value) const
        {
            SetUniform(GetUniformLocation(Name), Value);
        }

        void SetUniform(const int32_t UniformId, const glm::uvec4& Value) const
        {
            glProgramUniform4ui(_id, UniformId, Value.x, Value.y, Value.z, Value.w);
        }

        void SetUniform(const char* const Name, const glm::uvec4& Value) const
        {
            SetUniform(GetUniformLocation(Name), Value);
        }

        void SetUniform(const int32_t UniformId, const glm::mat2& Value) const
        {
            glProgramUniformMatrix2fv(_id, UniformId, 1, false, glm::value_ptr(Value));
        }

        void SetUniform(const char* const Name, const glm::mat2& Value) const
        {
            SetUniform(GetUniformLocation(Name), Value);
        }

        void SetUniform(const int32_t UniformId, const glm::mat2x3& Value) const
        {
            glProgramUniformMatrix2x3fv(_id, UniformId, 1, false, glm::value_ptr(Value));
        }

        void SetUniform(const char* const Name, const glm::mat2x3& Value) const
        {
            SetUniform(GetUniformLocation(Name), Value);
        }

        void SetUniform(const char* const Name, const glm::mat2x4& Value) const
        {
            SetUniform(GetUniformLocation(Name), Value);
        }

        void SetUniform(const int32_t UniformId, const glm::mat2x4& Value) const
        {
            glProgramUniformMatrix2x4fv(_id, UniformId, 1, false, glm::value_ptr(Value));
        }

        void SetUniform(const char* const Name, const glm::mat3& Value) const
        {
            SetUniform(GetUniformLocation(Name), Value);
        }

        void SetUniform(const int32_t UniformId, const glm::mat3& Value) const
        {
            glProgramUniformMatrix3fv(_id, UniformId, 1, false, glm::value_ptr(Value));
        }

        void SetUniform(const char* const Name, const glm::mat3x2& Value) const
        {
            SetUniform(GetUniformLocation(Name), Value);
        }

        void SetUniform(const int32_t UniformId, const glm::mat3x2& Value) const
        {
            glProgramUniformMatrix3x2fv(_id, UniformId, 1, false, glm::value_ptr(Value));
        }

        void SetUniform(const char* const Name, const glm::mat3x4& Value) const
        {
            SetUniform(GetUniformLocation(Name), Value);
        }

        void SetUniform(const int32_t UniformId, const glm::mat3x4& Value) const
        {
            glProgramUniformMatrix3x4fv(_id, UniformId, 1, false, glm::value_ptr(Value));
        }

        void SetUniform(const int32_t UniformId, const glm::mat4& Value) const
        {
            glProgramUniformMatrix4fv(_id, UniformId, 1, false, glm::value_ptr(Value));
        }

        void SetUniform(const char* const Name, const glm::mat4& Value) const
        {
            SetUniform(GetUniformLocation(Name), Value);
        }

        void SetUniformArray(const int32_t UniformId, const float* Array, const int32_t Count) const
        {
            glProgramUniform1fv(_id, UniformId, Count, Array);
        }

        void SetUniformArray(const char* const Name, const float* Array, const int32_t Count) const
        {
            glProgramUniform1fv(_id, GetUniformLocation(Name), Count, Array);
        }

        void SetUniformArray(const int32_t UniformId, const int32_t* Array, const int32_t Count) const
        {
            glProgramUniform1iv(_id, UniformId, Count, Array);
        }

        void SetUniformArray(const char* const Name, const int32_t* Array, const int32_t Count) const
        {
            glProgramUniform1iv(_id, GetUniformLocation(Name), Count, Array);
        }

        void SetUniformArray(const int32_t UniformId, const uint32_t* Array, const int32_t Count) const
        {
            glProgramUniform1uiv(_id, UniformId, Count, Array);
        }

        void SetUniformArray(const char* const Name, const uint32_t* Array, const int32_t Count) const
        {
            glProgramUniform1uiv(_id, GetUniformLocation(Name), Count, Array);
        }

        void SetUniform(const int32_t UniformId, const glm::mat4x2& Value) const
        {
            glProgramUniformMatrix4x2fv(_id, UniformId, 1, false, glm::value_ptr(Value));
        }

        void SetUniform(const char* const Name, const glm::mat4x2& Value) const
        {
            SetUniform(GetUniformLocation(Name), Value);
        }

        void SetUniform(const int32_t UniformId, const glm::mat4x3& Value) const
        {
            glProgramUniformMatrix4x3fv(_id, UniformId, 1, false, glm::value_ptr(Value));
        }

        void SetUniform(const char* const Name, const glm::mat4x3& Value) const
        {
            SetUniform(GetUniformLocation(Name), Value);
        }

        void SetTextureHandle(const int32_t UniformId, const uint64_t Value) const
        {
            glProgramUniformHandleui64ARB(_id, UniformId, Value);
        }

        void SetTextureHandle(const char* const Name, const uint64_t Value) const
        {
            SetTextureHandle(GetUniformLocation(Name), Value);
        }

        void SetUniformArray(const int32_t UniformId, const glm::vec2* Array, const int32_t Count) const
        {
            glProgramUniform2fv(_id, UniformId, Count, glm::value_ptr(*Array));
        }

        void SetUniformArray(const char* const Name, const glm::vec2* Array, const int32_t Count) const
        {
            glProgramUniform2fv(_id, GetUniformLocation(Name), Count, glm::value_ptr(*Array));
        }

        void SetUniformArray(const int32_t UniformId, const glm::vec3* Array, const int32_t Count) const
        {
            glProgramUniform3fv(_id, UniformId, Count, glm::value_ptr(*Array));
        }

        void SetUniformArray(const char* const Name, const glm::vec3* Array, const int32_t Count) const
        {
            glProgramUniform3fv(_id, GetUniformLocation(Name), Count, glm::value_ptr(*Array));
        }

        void SetUniformArray(const int32_t UniformId, const glm::vec4* Array, const int32_t Count) const
        {
            glProgramUniform4fv(_id, UniformId, Count, glm::value_ptr(*Array));
        }

        void SetUniformArray(const char* const Name, const glm::vec4* Array, const int32_t Count) const
        {
            glProgramUniform4fv(_id, GetUniformLocation(Name), Count, glm::value_ptr(*Array));
        }

        void SetUniformArray(const int32_t UniformId, const glm::ivec2* Array, const int32_t Count) const
        {
            glProgramUniform2iv(_id, UniformId, Count, glm::value_ptr(*Array));
        }

        void SetUniformArray(const char* const Name, const glm::ivec2* Array, const int32_t Count) const
        {
            glProgramUniform2iv(_id, GetUniformLocation(Name), Count, glm::value_ptr(*Array));
        }

        void SetUniformArray(const int32_t UniformId, const glm::ivec3* Array, const int32_t Count) const
        {
            glProgramUniform3iv(_id, UniformId, Count, glm::value_ptr(*Array));
        }

        void SetUniformArray(const char* const Name, const glm::ivec3* Array, const int32_t Count) const
        {
            glProgramUniform3iv(_id, GetUniformLocation(Name), Count, glm::value_ptr(*Array));
        }

        void SetUniformArray(const int32_t UniformId, const glm::ivec4* Array, const int32_t Count) const
        {
            glProgramUniform4iv(_id, UniformId, Count, glm::value_ptr(*Array));
        }

        void SetUniformArray(const char* const Name, const glm::ivec4* Array, const int32_t Count) const
        {
            glProgramUniform4iv(_id, GetUniformLocation(Name), Count, glm::value_ptr(*Array));
        }

        void SetUniformArray(const int32_t UniformId, const glm::uvec2* Array, const int32_t Count) const
        {
            glProgramUniform2uiv(_id, UniformId, Count, glm::value_ptr(*Array));
        }

        void SetUniformArray(const char* const Name, const glm::uvec2* Array, const int32_t Count) const
        {
            glProgramUniform2uiv(_id, GetUniformLocation(Name), Count, glm::value_ptr(*Array));
        }

        void SetUniformArray(const int32_t UniformId, const glm::uvec3* Array, const int32_t Count) const
        {
            glProgramUniform3uiv(_id, UniformId, Count, glm::value_ptr(*Array));
        }

        void SetUniformArray(const char* const Name, const glm::uvec3* Array, const int32_t Count) const
        {
            glProgramUniform3uiv(_id, GetUniformLocation(Name), Count, glm::value_ptr(*Array));
        }

        void SetUniformArray(const int32_t UniformId, const glm::uvec4* Array, const int32_t Count) const
        {
            glProgramUniform4uiv(_id, UniformId, Count, glm::value_ptr(*Array));
        }

        void SetUniformArray(const char* const Name, const glm::uvec4* Array, const int32_t Count) const
        {
            glProgramUniform4uiv(_id, GetUniformLocation(Name), Count, glm::value_ptr(*Array));
        }

        void SetUniformArray(const int32_t UniformId, const glm::mat2* Array, const int32_t Count) const
        {
            glProgramUniformMatrix2fv(_id, UniformId, Count, GL_FALSE, glm::value_ptr(*Array));
        }

        void SetUniformArray(const char* const Name, const glm::mat2* Array, const int32_t Count) const
        {
            glProgramUniformMatrix2fv(_id, GetUniformLocation(Name), Count, GL_FALSE, glm::value_ptr(*Array));
        }

        void SetUniformArray(const int32_t UniformId, const glm::mat2x3* Array, const int32_t Count) const
        {
            glProgramUniformMatrix2x3fv(_id, UniformId, Count, GL_FALSE, glm::value_ptr(*Array));
        }

        void SetUniformArray(const char* const Name, const glm::mat2x3* Array, const int32_t Count) const
        {
            glProgramUniformMatrix2x3fv(_id, GetUniformLocation(Name), Count, GL_FALSE, glm::value_ptr(*Array));
        }

        void SetUniformArray(const int32_t UniformId, const glm::mat2x4* Array, const int32_t Count) const
        {
            glProgramUniformMatrix2x4fv(_id, UniformId, Count, GL_FALSE, glm::value_ptr(*Array));
        }

        void SetUniformArray(const char* const Name, const glm::mat2x4* Array, const int32_t Count) const
        {
            glProgramUniformMatrix2x4fv(_id, GetUniformLocation(Name), Count, GL_FALSE, glm::value_ptr(*Array));
        }

        void SetUniformArray(const int32_t UniformId, const glm::mat3* Array, const int32_t Count) const
        {
            glProgramUniformMatrix3fv(_id, UniformId, Count, GL_FALSE, glm::value_ptr(*Array));
        }

        void SetUniformArray(const char* const Name, const glm::mat3* Array, const int32_t Count) const
        {
            glProgramUniformMatrix3fv(_id, GetUniformLocation(Name), Count, GL_FALSE, glm::value_ptr(*Array));
        }

        void SetUniformArray(const int32_t UniformId, const glm::mat3x2* Array, const int32_t Count) const
        {
            glProgramUniformMatrix3x2fv(_id, UniformId, Count, GL_FALSE, glm::value_ptr(*Array));
        }

        void SetUniformArray(const char* const Name, const glm::mat3x2* Array, const int32_t Count) const
        {
            glProgramUniformMatrix3x2fv(_id, GetUniformLocation(Name), Count, GL_FALSE, glm::value_ptr(*Array));
        }

        void SetUniformArray(const int32_t UniformId, const glm::mat3x4* Array, const int32_t Count) const
        {
            glProgramUniformMatrix3x4fv(_id, UniformId, Count, GL_FALSE, glm::value_ptr(*Array));
        }

        void SetUniformArray(const char* const Name, const glm::mat3x4* Array, const int32_t Count) const
        {
            glProgramUniformMatrix3x4fv(_id, GetUniformLocation(Name), Count, GL_FALSE, glm::value_ptr(*Array));
        }

        void SetUniformArray(const int32_t UniformId, const glm::mat4* Array, const int32_t Count) const
        {
            glProgramUniformMatrix4fv(_id, UniformId, Count, GL_FALSE, glm::value_ptr(*Array));
        }

        void SetUniformArray(const char* const Name, const glm::mat4* Array, const int32_t Count) const
        {
            glProgramUniformMatrix4fv(_id, GetUniformLocation(Name), Count, GL_FALSE, glm::value_ptr(*Array));
        }

        void SetUniformArray(const int32_t UniformId, const glm::mat4x2* Array, const int32_t Count) const
        {
            glProgramUniformMatrix4x2fv(_id, UniformId, Count, GL_FALSE, glm::value_ptr(*Array));
        }

        void SetUniformArray(const char* const Name, const glm::mat4x2* Array, const int32_t Count) const
        {
            glProgramUniformMatrix4x2fv(_id, GetUniformLocation(Name), Count, GL_FALSE, glm::value_ptr(*Array));
        }

        void SetUniformArray(const int32_t UniformId, const glm::mat4x3* Array, const int32_t Count) const
        {
            glProgramUniformMatrix4x3fv(_id, UniformId, Count, GL_FALSE, glm::value_ptr(*Array));
        }

        void SetUniformArray(const char* const Name, const glm::mat4x3* Array, const int32_t Count) const
        {
            glProgramUniformMatrix4x3fv(_id, GetUniformLocation(Name), Count, GL_FALSE, glm::value_ptr(*Array));
        }

    public:
        [[nodiscard]] friend bool operator==(const Shader& Rhs, const Shader& Lhs)
        {
            return Rhs._id == Lhs._id;
        }

        [[nodiscard]] friend bool operator!=(const Shader& Rhs, const Shader& Lhs)
        {
            return !(Rhs == Lhs);
        }
    };
}
