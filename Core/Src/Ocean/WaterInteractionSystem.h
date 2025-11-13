#pragma once
#include <glm/vec3.hpp>

#include "Meshes/Mesh.h"
#include "Ocean/Components/WaterEffectorComponent.h"
#include "Physics/PhysicsSystem.h"
#include "Physics/Components/RigidbodyComponent.h"
#include "Rendering/Shaders/Shader.h"
#include "Scene/Scene.h"

namespace Core
{
    struct TransformComponent;

    class WaterInteractionSystem
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
            uint32_t IntersectionPolygonsId;
            uint32_t Capacity;
            TriangleProperties* MappedOutputBuffer;
        };

        struct alignas(8) IntersectionSegment
        {
            glm::vec2 V0;
            glm::vec2 V1;
        };

        struct alignas(sizeof(float) * 4) ShaderMeshData
        {
            glm::mat4 ObjectToWorldMatrix;
            glm::vec3 ObjectVelocity;
            float DragCoefficient;
            uint32_t VerticesIndex;
            uint32_t IndicesIndex;
            uint32_t TrianglesCount;

            ShaderMeshData(const glm::mat4& ObjectToWorldMatrix,
                           const glm::vec3& ObjectVelocity,
                           const float DragCoefficient,
                           const uint32_t VerticesIndex, const uint32_t IndicesIndex,
                           const uint32_t TrianglesCount)
                : ObjectToWorldMatrix(ObjectToWorldMatrix),
                  ObjectVelocity(ObjectVelocity),
                  DragCoefficient(DragCoefficient),
                  VerticesIndex(VerticesIndex),
                  IndicesIndex(IndicesIndex),
                  TrianglesCount(TrianglesCount)
            {
            }
        };

        struct InteractiveWavesShaderUbo
        {
            glm::vec2 PreviousObjectWorldPosition0;
            glm::vec2 PreviousObjectWorldPosition1;
            glm::vec2 CurrentObjectWorldPosition;
            float InteractiveWavesFieldSize;
            float MaskFieldSize;
            float WaveDampingFactor;
            float AWaveFactor;
            float DisplacementStrength;
        };

        struct IntersectionMaskUbo
        {
            glm::vec2 ObjectWorldPosition;
            float InteractiveWavesFieldSize;
        };

    private:
        TriangleBuffer _triangleBuffer;
        Shader _triangleProcessorComputeShader;
        Shader _triangleAggregationComputeShader;
        Shader _outputComputeShader;
        Shader _intersectionMaskComputeShader;
        Shader _interactiveWavesComputeShader;
        uint32_t _meshDataUbo;
        uint32_t _intersectionMaskUbo;
        uint32_t _interactiveWavesUbo;
        float _dragCoefficient = 2.02f;

        int32_t _windVelocityLocation;
        int32_t _triangleCountLocation;
        int32_t _partialsCountLocation;

    public:
        WaterInteractionSystem();

    public:
        ~WaterInteractionSystem();

        void ProcessMeshes(Scene& Scene, PhysicsSystem& PhysicsSystem, uint32_t DisplacementTextures,
                           glm::vec2 WindVelocity, uint32_t VelocityTextures, uint32_t CascadeSettingsBuffer,
                           float DeltaTime);

    private:
        void ProcessMesh(PhysicsSystem& PhysicsSystem, WaterEffectorComponent& WaterEffectorComponent,
                         const TransformComponent& Transform, const RigidbodyComponent& Rigidbody,
                         uint32_t CascadeSettingsBuffer, float DeltaTime);

        void DispatchTriangleProcessing(const Mesh& Mesh, uint32_t TriangleCount, const glm::vec3& MeshVelocity,
                                        const glm::mat4& LocalToWorldMatrix, uint32_t CascadeSettingsBuffer) const;

        void DispatchTriangleAggregation(uint32_t TriangleCount,
                                         int32_t AggregationGroupsCount) const;

        void DispatchOutputProcessing(uint32_t AggregationGroupsCount) const;

        void DispatchIntersectionMaskCompute(WaterEffectorComponent& WaterEffectorComponent,
                                             const glm::vec3& WorldPosition) const;

        void DispatchInteractiveWavesCompute(WaterEffectorComponent& WaterEffectorComponent,
                                             const glm::vec3& ObjectVelocity, const glm::vec3& WorldPosition,
                                             float DeltaTime) const;

        void ApplyForces(PhysicsSystem& PhysicsSystem, const RigidbodyComponent& Rigidbody,
                         const glm::mat4& LocalToWorldMatrix, const glm::vec3& WorldPosition) const;

        void ResizeTriangleBuffers(uint32_t TriangleCount);

        void ResetIntersectionPolygonsBuffer() const;

        static TriangleBuffer CreateTriangleBuffers();

        static uint32_t CreateMeshDataUbo();

        static uint32_t CreateIntersectionMaskUbo();

        static uint32_t CreateInteractiveWavesUbo();
    };
}
