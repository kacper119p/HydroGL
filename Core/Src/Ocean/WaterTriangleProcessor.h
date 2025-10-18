#pragma once
#include <glm/vec3.hpp>

#include "Meshes/Mesh.h"
#include "Rendering/Shaders/Shader.h"
#include "Scene/Scene.h"

namespace Core
{
    struct TransformComponent;

    class WaterTriangleProcessor
    {
    private:
        struct alignas(sizeof(float) * 4) TriangleProperties
        {
            glm::vec3 Drag;
            float Padding0;
            float SubmergedArea;
            float SubmergedVolume;
            float Padding1[2];
        };

        struct TriangleBuffer
        {
            uint32_t MainBufferId;
            uint32_t PartialSumsBufferId;
            uint32_t OutputBufferId;
            uint32_t Capacity;
            TriangleProperties* MappedOutputBuffer;
        };

        struct ShaderMeshData
        {
            glm::mat4 ObjectToWorldMatrix;
            uint32_t VerticesIndex;
            uint32_t IndicesIndex;
            uint32_t TrianglesCount;
            float Padding;
        };

    private:
        TriangleBuffer _triangleBuffer;
        Shader _triangleProcessorComputeShader;
        Shader _triangleAggregationComputeShader;
        Shader _outputComputeShader;
        uint32_t _meshDataUbo;

    public:
        WaterTriangleProcessor();

    public:
        void ProcessMeshes(Scene& Scene, uint32_t DisplacementTextures) const;

    private:
        void ProcessMesh(const Mesh& Mesh, const TransformComponent& Transform) const;

        void ResizeTriangleBuffers(uint32_t TriangleCount) const;

        static TriangleBuffer CreateTriangleBuffers();

        static uint32_t CreateMeshDataUbo();
    };
}
