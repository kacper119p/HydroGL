#pragma once
#include <glm/vec3.hpp>

#include "Meshes/Mesh.h"
#include "Physics/PhysicsSystem.h"
#include "Physics/Components/RigidbodyComponent.h"
#include "Rendering/Shaders/Shader.h"
#include "Scene/Scene.h"

namespace Core
{
    struct TransformComponent;

    class BuoyancySystem
    {
    private:
        struct alignas(sizeof(float) * 4) TriangleProperties
        {
            glm::vec3 Drag;
            float SubmergedArea;
            glm::vec3 CenterOfImmersion;
            float SubmergedVolume;
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
        BuoyancySystem();

    public:
        void ProcessMeshes(Scene& Scene, PhysicsSystem& PhysicsSystem, uint32_t DisplacementTextures,
                           uint32_t CascadeSettingsBuffer);

    private:
        void ProcessMesh(PhysicsSystem& PhysicsSystem, const Mesh& Mesh, const TransformComponent& Transform,
                         const RigidbodyComponent& Rigidbody,
                         uint32_t CascadeSettingsBuffer);

        void ResizeTriangleBuffers(uint32_t TriangleCount);

        static TriangleBuffer CreateTriangleBuffers();

        static uint32_t CreateMeshDataUbo();
    };
}
