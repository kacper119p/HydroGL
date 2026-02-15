#pragma once
#include <glm/vec3.hpp>

#include "Meshes/Mesh.h"
#include "Ocean/Components/WaterEffectorComponent.h"
#include "Physics/PhysicsSystem.h"
#include "Physics/Components/RigidbodyComponent.h"
#include "Rendering/Shaders/Shader.h"
#include "Rendering/Std140/Mat2.h"
#include "Scene/Scene.h"
#include "Utility/GlResourceUnique.h"

namespace Core
{
    class WaterInteractionSystem
    {
    private:
        using ComponentsViewType = entt::basic_view<entt::get_t<entt::basic_sigh_mixin<entt::basic_storage<
            WaterEffectorComponent>, entt::basic_registry<>>, entt::basic_sigh_mixin<entt::basic_storage<
            TransformComponent>, entt::basic_registry<>>, entt::basic_sigh_mixin<entt::basic_storage<RigidbodyComponent>
            , entt::basic_registry<>>>, entt::exclude_t<>>;

        struct alignas(16) TriangleProperties
        {
            alignas(16) glm::vec3 WaterDrag;
            alignas(4) float SubmergedArea;
            alignas(16) glm::vec3 CenterOfImmersion;
            alignas(4) float SubmergedVolume;
            alignas(16) glm::vec3 CenterOfAir;
            alignas(4) float AirVolume;
            alignas(16) glm::vec3 AirDrag;
        };

        struct TriangleBuffer
        {
            GlBufferUnique MainBuffer;
            GlBufferUnique PartialSumsBuffer;
            GlBufferUnique OutputBuffer;
            GlBufferUnique IntersectionPolygons;
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
            alignas(8) glm::vec2 PreviousObjectWorldPosition1;
            alignas(8) glm::vec2 CurrentObjectWorldPosition;
            alignas(4) float PreviousInteractiveWavesFieldSize0;
            alignas(4) float PreviousInteractiveWavesFieldSize1;
            alignas(4) float CurrentInteractiveWavesFieldSize;
            alignas(4) float MaskFieldSize;
            alignas(4) float WaveDampingFactor;
            alignas(4) float AWaveFactor;
            alignas(4) float DisplacementStrength;
        };

        struct IntersectionMaskUbo
        {
            Std140::Mat2 VelocitySpaceTransform;
            alignas(16) glm::vec2 ObjectWorldPosition;
            alignas(4) float IntersectionMaskPhysicalSize;
            float BoundsSizeX;
            float Ga;
            float Gb;
            alignas(4) float DisplacementStrength;
            alignas(4) float DeltaTime;
        };

        struct alignas(16) DynamicWaveSampleParameters
        {
            glm::vec3 ObjectPosition;
            float WaveFieldSize;
            uint64_t WaveFieldTexture;
            uint32_t WaveFieldTextureResolution;
        };

        struct alignas(16) DynamicWavesSampleSsbo
        {
            int32_t DynamicWavesCount;
            float Padding[3];
            DynamicWaveSampleParameters DynamicWaves[1];
        };

        struct DynamicWavesSampleBuffer
        {
            GlBufferUnique GpuBuffer;
            int32_t Capacity;
            DynamicWavesSampleSsbo* DynamicWavesBuffer;
        };

    private:
        TriangleBuffer _triangleBuffer;
        DynamicWavesSampleBuffer _dynamicWavesSampleBuffer;
        Shader _triangleProcessorComputeShader;
        Shader _triangleAggregationComputeShader;
        Shader _outputComputeShader;
        Shader _intersectionMaskComputeShader;
        Shader _interactiveWavesComputeShader;
        GlBufferUnique _meshDataUbo;
        GlBufferUnique _intersectionMaskUbo;
        GlBufferUnique _interactiveWavesUbo;

        int32_t _windVelocityLocation;
        int32_t _triangleCountLocation;
        int32_t _partialsCountLocation;

    public:
        /**
         * @brief Default constructor.
         */
        WaterInteractionSystem();

    public:
        /**
         * @brief Destructor.
         */
        ~WaterInteractionSystem();

    public:
        /**
         * @brief Gets the OpenGL ID of the Shader Storage Buffer Object (SSBO) containing dynamic wave sample data.
         * @return The OpenGL buffer ID.
         */
        [[nodiscard]] uint32_t GetDynamicWavesSampleBufferId() const
        {
            return _dynamicWavesSampleBuffer.GpuBuffer;
        }

        /**
         * @brief Gets the number of current dynamic waves.
         * @return The count of dynamic waves.
         */
        [[nodiscard]] int32_t GetDynamicWavesSampleCount() const
        {
            return _dynamicWavesSampleBuffer.DynamicWavesBuffer->DynamicWavesCount;
        }

    public:
        /**
         * @brief Evaluates interactions for all valid entities and applies the resulting physical forces.
         * @param Scene The scene containing entities.
         * @param PhysicsSystem The physics system to which computed forces will be applied.
         * @param DisplacementTextures OpenGL texture array ID containing water displacement data.
         * @param WindVelocity The current global 2D wind velocity vector.
         * @param VelocityTextures OpenGL texture array ID containing water surface velocity data.
         * @param CascadeSettingsBuffer OpenGL SSBO ID containing water cascade configurations.
         * @param DeltaTime The elapsed time since the last frame in seconds.
         */
        void ProcessMeshes(Scene& Scene, PhysicsSystem& PhysicsSystem, uint32_t DisplacementTextures,
                           glm::vec2 WindVelocity, uint32_t VelocityTextures, uint32_t CascadeSettingsBuffer,
                           float DeltaTime);

    private:
        void ProcessMesh(PhysicsSystem& PhysicsSystem, WaterEffectorComponent& WaterEffectorComponent,
                         const TransformComponent& Transform, const RigidbodyComponent& Rigidbody,
                         uint32_t CascadeSettingsBuffer, float DeltaTime);

        void DispatchTriangleProcessing(const Mesh& Mesh, uint32_t TriangleCount,
                                        const glm::vec3& MeshVelocity,
                                        const glm::mat4& LocalToWorldMatrix,
                                        uint32_t CascadeSettingsBuffer,
                                        float DragCoefficient) const;

        void DispatchTriangleAggregation(uint32_t TriangleCount,
                                         int32_t AggregationGroupsCount) const;

        void DispatchOutputProcessing(uint32_t AggregationGroupsCount) const;

        void DispatchIntersectionMaskCompute(WaterEffectorComponent& WaterEffectorComponent,
                                             const glm::vec3& WorldPosition,
                                             const glm::vec3& ObjectVelocity,
                                             const glm::mat4& LocalToWorldMatrix,
                                             float SubmergedVolume,
                                             float DeltaTime) const;

        void DispatchInteractiveWavesCompute(WaterEffectorComponent& WaterEffectorComponent,
                                             const glm::vec3& ObjectVelocity, const glm::vec3& WorldPosition,
                                             float DeltaTime) const;

        void ApplyForces(PhysicsSystem& PhysicsSystem, const RigidbodyComponent& Rigidbody,
                         const glm::mat4& LocalToWorldMatrix, const glm::vec3& WorldPosition,
                         float& SubmergedVolume) const;

        void ResizeTriangleBuffers(uint32_t TriangleCount);

        void ResetIntersectionPolygonsBuffer() const;

        void ResizeDynamicWavesSampleSsbo(uint32_t Count);

        void UpdateDynamicWavesSampleSsbo(const ComponentsViewType& View);

        static TriangleBuffer CreateTriangleBuffers();

        static uint32_t CreateMeshDataUbo();

        static uint32_t CreateIntersectionMaskUbo();

        static uint32_t CreateInteractiveWavesUbo();

        static DynamicWavesSampleBuffer CreateDynamicWavesSampleBuffer();
    };
}
