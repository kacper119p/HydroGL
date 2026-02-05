#pragma once
#include <cstdint>
#include <vector>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include "Camera/Frustum.h"
#include "Utility/GlResourceUnique.h"

namespace Core
{
    class OceanGrid
    {
    private:
        static constexpr uint32_t PatchSubdivisions = 8;
        static constexpr int32_t LodLevels = 10;
        static constexpr float MinLodDistance = 150.0f;
        static constexpr float GridSize = 20000.0f;

    private:
        float _lodDistances[LodLevels];

        GlVertexArrayUnique _nearVao;
        GlVertexArrayUnique _farVao;
        GlBufferUnique _vbo;
        GlBufferUnique _patchInstanceDataNearVbo;
        GlBufferUnique _patchInstanceDataFarVbo;
        uint32_t _patchInstanceDataNearVboSize{};
        uint32_t _patchInstanceDataFarVboSize{};
        GlBufferUnique _nearEbo;
        GlBufferUnique _farEbo;
        int32_t _nearElementCount;
        int32_t _farElementCount;
        float _quantizationSize;

        std::vector<glm::vec4> _patchesFar;
        std::vector<glm::vec4> _patchesNear;

        bool _renderAsWireframe = false;

    public:
        [[nodiscard]] static constexpr uint32_t GetPatchSubdivisions()
        {
            return PatchSubdivisions;
        }

        [[nodiscard]] float GetMinLodDistance() const
        {
            return _lodDistances[0];
        }

        [[nodiscard]] bool GetRenderAsWireframe() const
        {
            return _renderAsWireframe;
        }

        void SetRenderAsWireframe(const bool RenderAsWireframe)
        {
            _renderAsWireframe = RenderAsWireframe;
        }

    public:
        explicit OceanGrid();

    public:
        ~OceanGrid();

    private:
        static GlBufferUnique CreateVbo(uint32_t VerticesOnEdge, float QuadSize);

    public:
        void UpdateBuffers(const glm::mat4& ProjectionMatrix, const glm::mat4& ViewMatrix,
                           const glm::vec3& CameraPosition);

        void RenderNearPatches() const;

        void RenderFarPatches() const;

    private:
        bool SelectNode(const glm::vec3& CameraPosition, glm::vec2 NodePosition, float NodeSize, int32_t Level,
                        const Frustum& Frustum);

        [[nodiscard]] bool IsNodeInRange(const glm::vec3& CameraPosition, glm::vec2 NodePosition, float NodeSize,
                                         int32_t Level) const;


        void InitializeNearVao(uint32_t VerticesOnEdge);

        void InitializeFarVao(uint32_t VerticesOnEdge);
    };
}
