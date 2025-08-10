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
        uint32_t Id;

    public:
        Shader() : Id(0)
        {
        }

        explicit Shader(const uint32_t ProgramId) : Id(ProgramId)
        {
        }

        Shader(const Shader& Other) noexcept : Id(Other.Id)
        {
        }

    public:
        [[nodiscard]] static glm::ivec3 GetComputeShaderWorkGroupSize(const Shader& Shader)
        {
            int localWorkGroupSize[3];
            glGetProgramiv(Shader.GetId(), GL_COMPUTE_WORK_GROUP_SIZE, localWorkGroupSize);
            return glm::ivec3(localWorkGroupSize[0], localWorkGroupSize[1], localWorkGroupSize[2]);
        }

        static void DispatchCompute(const glm::ivec3& WorkGroupCount)
        {
            glDispatchCompute(WorkGroupCount.x, WorkGroupCount.y, WorkGroupCount.z);
        }

        static void DispatchCompute()
        {
            glDispatchCompute(1, 1, 1);
        }

    public:
        void Use() const
        {
            glUseProgram(Id);
        }

        [[nodiscard]] bool IsNull() const
        {
            return Id == 0;
        }

        [[nodiscard]] unsigned int GetId() const
        {
            return Id;
        }

        int32_t GetUniformLocation(const char* const Name) const
        {
            return glGetUniformLocation(Id, Name);
        }

        static void SetUniform(const int32_t UniformId, const float Value)
        {
            glUniform1f(UniformId, Value);
        }

        void SetUniform(const char* const Name, const float Value) const
        {
            SetUniform(GetUniformLocation(Name), Value);
        }

        static void SetUniform(const int32_t UniformId, const glm::vec2& Value)
        {
            glUniform2f(UniformId, Value.x, Value.y);
        }

        void SetUniform(const char* const Name, const glm::vec2& Value) const
        {
            SetUniform(GetUniformLocation(Name), Value);
        }

        static void SetUniform(const int32_t UniformId, const glm::vec3& Value)
        {
            glUniform3f(UniformId, Value.x, Value.y, Value.z);
        }

        void SetUniform(const char* const Name, const glm::vec3& Value) const
        {
            SetUniform(GetUniformLocation(Name), Value);
        }

        static void SetUniform(const int32_t UniformId, const glm::vec4& Value)
        {
            glUniform4f(UniformId, Value.x, Value.y, Value.z, Value.w);
        }

        void SetUniform(const char* const Name, const glm::vec4& Value) const
        {
            SetUniform(GetUniformLocation(Name), Value);
        }

        static void SetUniform(const int32_t UniformId, const int32_t Value)
        {
            glUniform1i(UniformId, Value);
        }

        void SetUniform(const char* const Name, const int32_t Value) const
        {
            SetUniform(GetUniformLocation(Name), Value);
        }

        static void SetUniform(const int32_t UniformId, const glm::ivec2& Value)
        {
            glUniform2i(UniformId, Value.x, Value.y);
        }

        void SetUniform(const char* const Name, const glm::ivec2& Value) const
        {
            SetUniform(GetUniformLocation(Name), Value);
        }

        static void SetUniform(const int32_t UniformId, const glm::ivec3& Value)
        {
            glUniform3i(UniformId, Value.x, Value.y, Value.z);
        }

        void SetUniform(const char* const Name, const glm::ivec3& Value) const
        {
            SetUniform(GetUniformLocation(Name), Value);
        }

        static void SetUniform(const int32_t UniformId, const glm::ivec4& Value)
        {
            glUniform4i(UniformId, Value.x, Value.y, Value.z, Value.w);
        }

        void SetUniform(const char* const Name, const glm::ivec4& Value) const
        {
            SetUniform(GetUniformLocation(Name), Value);
        }

        static void SetUniform(const int32_t UniformId, const bool Value)
        {
            glUniform1i(UniformId, static_cast<int>(Value));
        }

        void SetUniform(const char* const Name, const bool Value) const
        {
            SetUniform(GetUniformLocation(Name), Value);
        }

        static void SetUniform(const int32_t UniformId, const uint32_t Value)
        {
            glUniform1ui(UniformId, Value);
        }

        void SetUniform(const char* const Name, const uint32_t Value) const
        {
            SetUniform(GetUniformLocation(Name), Value);
        }

        static void SetUniform(const int32_t UniformId, const glm::uvec2 Value)
        {
            glUniform2ui(UniformId, Value.x, Value.y);
        }

        void SetUniform(const char* const Name, const glm::uvec2& Value) const
        {
            SetUniform(GetUniformLocation(Name), Value);
        }

        static void SetUniform(const int32_t UniformId, const glm::uvec3& Value)
        {
            glUniform3ui(UniformId, Value.x, Value.y, Value.z);
        }

        void SetUniform(const char* const Name, const glm::uvec3& Value) const
        {
            SetUniform(GetUniformLocation(Name), Value);
        }

        static void SetUniform(const int32_t UniformId, const glm::uvec4& Value)
        {
            glUniform4ui(UniformId, Value.x, Value.y, Value.z, Value.w);
        }

        void SetUniform(const char* const Name, const glm::uvec4& Value) const
        {
            SetUniform(GetUniformLocation(Name), Value);
        }

        static void SetUniform(const int32_t UniformId, const glm::mat2& Value)
        {
            glUniformMatrix2fv(UniformId, 1, false, glm::value_ptr(Value));
        }

        void SetUniform(const char* const Name, const glm::mat2& Value) const
        {
            SetUniform(GetUniformLocation(Name), Value);
        }

        static void SetUniform(const int32_t UniformId, const glm::mat2x3& Value)
        {
            glUniformMatrix2x3fv(UniformId, 1, false, glm::value_ptr(Value));
        }

        void SetUniform(const char* const Name, const glm::mat2x3& Value) const
        {
            SetUniform(GetUniformLocation(Name), Value);
        }

        void SetUniform(const char* const Name, const glm::mat2x4& Value) const
        {
            SetUniform(GetUniformLocation(Name), Value);
        }

        static void SetUniform(const int32_t UniformId, const glm::mat2x4& Value)
        {
            glUniformMatrix2x4fv(UniformId, 1, false, glm::value_ptr(Value));
        }

        void SetUniform(const char* const Name, const glm::mat3& Value) const
        {
            SetUniform(GetUniformLocation(Name), Value);
        }

        static void SetUniform(const int32_t UniformId, const glm::mat3& Value)
        {
            glUniformMatrix3fv(UniformId, 1, false, glm::value_ptr(Value));
        }

        void SetUniform(const char* const Name, const glm::mat3x2& Value) const
        {
            SetUniform(GetUniformLocation(Name), Value);
        }

        static void SetUniform(const int32_t UniformId, const glm::mat3x2& Value)
        {
            glUniformMatrix3x2fv(UniformId, 1, false, glm::value_ptr(Value));
        }

        void SetUniform(const char* const Name, const glm::mat3x4& Value) const
        {
            SetUniform(GetUniformLocation(Name), Value);
        }

        static void SetUniform(const int32_t UniformId, const glm::mat3x4& Value)
        {
            glUniformMatrix3x4fv(UniformId, 1, false, glm::value_ptr(Value));
        }

        static void SetUniform(const int32_t UniformId, const glm::mat4& Value)
        {
            glUniformMatrix4fv(UniformId, 1, false, glm::value_ptr(Value));
        }

        void SetUniform(const char* const Name, const glm::mat4& Value) const
        {
            SetUniform(GetUniformLocation(Name), Value);
        }

        static void SetUniformArray(const int32_t UniformId, const float* Array, const int32_t Count)
        {
            glUniform1fv(UniformId, Count, Array);
        }

        void SetUniformArray(const char* const Name, const float* Array, const int32_t Count) const
        {
            glUniform1fv(GetUniformLocation(Name), Count, Array);
        }

        static void SetUniformArray(const int32_t UniformId, const int32_t* Array, const int32_t Count)
        {
            glUniform1iv(UniformId, Count, Array);
        }

        void SetUniformArray(const char* const Name, const int32_t* Array, const int32_t Count) const
        {
            glUniform1iv(GetUniformLocation(Name), Count, Array);
        }

        static void SetUniformArray(const int32_t UniformId, const uint32_t* Array, const int32_t Count)
        {
            glUniform1uiv(UniformId, Count, Array);
        }

        void SetUniformArray(const char* const Name, const uint32_t* Array, const int32_t Count) const
        {
            glUniform1uiv(GetUniformLocation(Name), Count, Array);
        }

        static void SetUniform(const int32_t UniformId, const glm::mat4x2& Value)
        {
            glUniformMatrix4x2fv(UniformId, 1, false, glm::value_ptr(Value));
        }

        void SetUniform(const char* const Name, const glm::mat4x2& Value) const
        {
            SetUniform(GetUniformLocation(Name), Value);
        }

        static void SetUniform(const int32_t UniformId, const glm::mat4x3& Value)
        {
            glUniformMatrix4x3fv(UniformId, 1, false, glm::value_ptr(Value));
        }

        void SetUniform(const char* const Name, const glm::mat4x3& Value) const
        {
            SetUniform(GetUniformLocation(Name), Value);
        }

        static void SetTextureHandle(const int32_t UniformId, const uint64_t Value)
        {
            glUniformHandleui64ARB(UniformId, Value);
        }

        void SetTextureHandle(const char* const Name, const uint64_t Value) const
        {
            SetTextureHandle(GetUniformLocation(Name), Value);
        }

        static void SetUniformArray(const int32_t UniformId, const glm::vec2* Array, const int32_t Count)
        {
            glUniform2fv(UniformId, Count, glm::value_ptr(*Array));
        }

        void SetUniformArray(const char* const Name, const glm::vec2* Array, const int32_t Count) const
        {
            glUniform2fv(GetUniformLocation(Name), Count, glm::value_ptr(*Array));
        }

        static void SetUniformArray(const int32_t UniformId, const glm::vec3* Array, const int32_t Count)
        {
            glUniform3fv(UniformId, Count, glm::value_ptr(*Array));
        }

        void SetUniformArray(const char* const Name, const glm::vec3* Array, const int32_t Count) const
        {
            glUniform3fv(GetUniformLocation(Name), Count, glm::value_ptr(*Array));
        }

        static void SetUniformArray(const int32_t UniformId, const glm::vec4* Array, const int32_t Count)
        {
            glUniform4fv(UniformId, Count, glm::value_ptr(*Array));
        }

        void SetUniformArray(const char* const Name, const glm::vec4* Array, const int32_t Count) const
        {
            glUniform4fv(GetUniformLocation(Name), Count, glm::value_ptr(*Array));
        }

        static void SetUniformArray(const int32_t UniformId, const glm::ivec2* Array, const int32_t Count)
        {
            glUniform2iv(UniformId, Count, glm::value_ptr(*Array));
        }

        void SetUniformArray(const char* const Name, const glm::ivec2* Array, const int32_t Count) const
        {
            glUniform2iv(GetUniformLocation(Name), Count, glm::value_ptr(*Array));
        }

        static void SetUniformArray(const int32_t UniformId, const glm::ivec3* Array, const int32_t Count)
        {
            glUniform3iv(UniformId, Count, glm::value_ptr(*Array));
        }

        void SetUniformArray(const char* const Name, const glm::ivec3* Array, const int32_t Count) const
        {
            glUniform3iv(GetUniformLocation(Name), Count, glm::value_ptr(*Array));
        }

        static void SetUniformArray(const int32_t UniformId, const glm::ivec4* Array, const int32_t Count)
        {
            glUniform4iv(UniformId, Count, glm::value_ptr(*Array));
        }

        void SetUniformArray(const char* const Name, const glm::ivec4* Array, const int32_t Count) const
        {
            glUniform4iv(GetUniformLocation(Name), Count, glm::value_ptr(*Array));
        }

        static void SetUniformArray(const int32_t UniformId, const glm::uvec2* Array, const int32_t Count)
        {
            glUniform2uiv(UniformId, Count, glm::value_ptr(*Array));
        }

        void SetUniformArray(const char* const Name, const glm::uvec2* Array, const int32_t Count) const
        {
            glUniform2uiv(GetUniformLocation(Name), Count, glm::value_ptr(*Array));
        }

        static void SetUniformArray(const int32_t UniformId, const glm::uvec3* Array, const int32_t Count)
        {
            glUniform3uiv(UniformId, Count, glm::value_ptr(*Array));
        }

        void SetUniformArray(const char* const Name, const glm::uvec3* Array, const int32_t Count) const
        {
            glUniform3uiv(GetUniformLocation(Name), Count, glm::value_ptr(*Array));
        }

        static void SetUniformArray(const int32_t UniformId, const glm::uvec4* Array, const int32_t Count)
        {
            glUniform4uiv(UniformId, Count, glm::value_ptr(*Array));
        }

        void SetUniformArray(const char* const Name, const glm::uvec4* Array, const int32_t Count) const
        {
            glUniform4uiv(GetUniformLocation(Name), Count, glm::value_ptr(*Array));
        }

        static void SetUniformArray(const int32_t UniformId, const glm::mat2* Array, const int32_t Count)
        {
            glUniformMatrix2fv(UniformId, Count, GL_FALSE, glm::value_ptr(*Array));
        }

        void SetUniformArray(const char* const Name, const glm::mat2* Array, const int32_t Count) const
        {
            glUniformMatrix2fv(GetUniformLocation(Name), Count, GL_FALSE, glm::value_ptr(*Array));
        }

        static void SetUniformArray(const int32_t UniformId, const glm::mat2x3* Array, const int32_t Count)
        {
            glUniformMatrix2x3fv(UniformId, Count, GL_FALSE, glm::value_ptr(*Array));
        }

        void SetUniformArray(const char* const Name, const glm::mat2x3* Array, const int32_t Count) const
        {
            glUniformMatrix2x3fv(GetUniformLocation(Name), Count, GL_FALSE, glm::value_ptr(*Array));
        }

        static void SetUniformArray(const int32_t UniformId, const glm::mat2x4* Array, const int32_t Count)
        {
            glUniformMatrix2x4fv(UniformId, Count, GL_FALSE, glm::value_ptr(*Array));
        }

        void SetUniformArray(const char* const Name, const glm::mat2x4* Array, const int32_t Count) const
        {
            glUniformMatrix2x4fv(GetUniformLocation(Name), Count, GL_FALSE, glm::value_ptr(*Array));
        }

        static void SetUniformArray(const int32_t UniformId, const glm::mat3* Array, const int32_t Count)
        {
            glUniformMatrix3fv(UniformId, Count, GL_FALSE, glm::value_ptr(*Array));
        }

        void SetUniformArray(const char* const Name, const glm::mat3* Array, const int32_t Count) const
        {
            glUniformMatrix3fv(GetUniformLocation(Name), Count, GL_FALSE, glm::value_ptr(*Array));
        }

        static void SetUniformArray(const int32_t UniformId, const glm::mat3x2* Array, const int32_t Count)
        {
            glUniformMatrix3x2fv(UniformId, Count, GL_FALSE, glm::value_ptr(*Array));
        }

        void SetUniformArray(const char* const Name, const glm::mat3x2* Array, const int32_t Count) const
        {
            glUniformMatrix3x2fv(GetUniformLocation(Name), Count, GL_FALSE, glm::value_ptr(*Array));
        }

        static void SetUniformArray(const int32_t UniformId, const glm::mat3x4* Array, const int32_t Count)
        {
            glUniformMatrix3x4fv(UniformId, Count, GL_FALSE, glm::value_ptr(*Array));
        }

        void SetUniformArray(const char* const Name, const glm::mat3x4* Array, const int32_t Count) const
        {
            glUniformMatrix3x4fv(GetUniformLocation(Name), Count, GL_FALSE, glm::value_ptr(*Array));
        }

        static void SetUniformArray(const int32_t UniformId, const glm::mat4* Array, const int32_t Count)
        {
            glUniformMatrix4fv(UniformId, Count, GL_FALSE, glm::value_ptr(*Array));
        }

        void SetUniformArray(const char* const Name, const glm::mat4* Array, const int32_t Count) const
        {
            glUniformMatrix4fv(GetUniformLocation(Name), Count, GL_FALSE, glm::value_ptr(*Array));
        }

        static void SetUniformArray(const int32_t UniformId, const glm::mat4x2* Array, const int32_t Count)
        {
            glUniformMatrix4x2fv(UniformId, Count, GL_FALSE, glm::value_ptr(*Array));
        }

        void SetUniformArray(const char* const Name, const glm::mat4x2* Array, const int32_t Count) const
        {
            glUniformMatrix4x2fv(GetUniformLocation(Name), Count, GL_FALSE, glm::value_ptr(*Array));
        }

        static void SetUniformArray(const int32_t UniformId, const glm::mat4x3* Array, const int32_t Count)
        {
            glUniformMatrix4x3fv(UniformId, Count, GL_FALSE, glm::value_ptr(*Array));
        }

        void SetUniformArray(const char* const Name, const glm::mat4x3* Array, const int32_t Count) const
        {
            glUniformMatrix4x3fv(GetUniformLocation(Name), Count, GL_FALSE, glm::value_ptr(*Array));
        }

    public:
        bool operator==(const Shader& Other) const
        {
            return Other.Id == this->Id;
        }

        bool operator!=(const Shader& Other) const
        {
            return !(*this == Other);
        }
    };
} // namespace Core
