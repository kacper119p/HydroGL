#pragma once
#include <cstdint>
#include <vector>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include "Camera/Frustum.h"
#include "Utility/GlResourceUnique.h"

namespace Core
{
    /**
     * @brief Represents an adaptive ocean grid.
     */
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
        /**
         * @brief Gets the number of subdivisions per ocean patch.
         * @return The subdivision count.
         */
        [[nodiscard]] static constexpr uint32_t GetPatchSubdivisions()
        {
            return PatchSubdivisions;
        }

        /**
         * @brief Gets the minimum distance for the highest Level of Detail (LOD).
         * @return The minimum LOD distance in world units.
         */
        [[nodiscard]] float GetMinLodDistance() const
        {
            return _lodDistances[0];
        }

        /**
         * @brief Checks if the grid is set to be rendered in wireframe mode.
         * @return True if wireframe rendering is enabled, false otherwise.
         */
        [[nodiscard]] bool GetRenderAsWireframe() const
        {
            return _renderAsWireframe;
        }

        /**
         * @brief Sets the wireframe rendering mode for the ocean grid.
         * @param RenderAsWireframe True to enable wireframe rendering, false to disable.
         */
        void SetRenderAsWireframe(const bool RenderAsWireframe)
        {
            _renderAsWireframe = RenderAsWireframe;
        }

    public:
        /**
         * @brief Default constructor.
         * Initializes OpenGL resources.
         */
        explicit OceanGrid();

    public:
        /**
         * @brief Destructor.
         */
        ~OceanGrid();

    private:
        static GlBufferUnique CreateVbo(uint32_t VerticesOnEdge, float QuadSize);

    public:
        /**
         * @brief Updates the data buffers for the current frame.
         * @param ProjectionMatrix The camera's projection matrix.
         * @param ViewMatrix The camera's view matrix.
         * @param CameraPosition The current position of the camera in world space.
         */
        void UpdateBuffers(const glm::mat4& ProjectionMatrix, const glm::mat4& ViewMatrix,
                           const glm::vec3& CameraPosition);

        /**
         * @brief Renders the LOD 0 ocean patches.
         */
        void RenderNearPatches() const;

        /**
         * @brief Renders the LOD > 1 ocean patches.
         */
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
