#pragma once
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "Collections/CyclicBuffer.h"
#include "Meshes/Mesh.h"

namespace Core
{
    /**
     * @brief A component that allows an entity to interact physically and visually with the water surface.
     */
    class WaterEffectorComponent
    {
    public:
        /**
         * @brief The geometric representation used for intersection, volume, and drag calculations.
         */
        Mesh Mesh;

        /**
         * @brief OpenGL texture ID storing the 2D intersection mask between the mesh and the water surface.
         */
        uint32_t MaskTexture;

        /**
         * @brief Cyclic buffer of OpenGL texture IDs storing the dynamic wave heightmaps over recent frames.
         */
        CyclicBuffer<uint32_t, 3> InteractiveWavesTextures;

        /**
         * @brief Cyclic buffer of OpenGL bindless texture handles corresponding to the wave textures.
         */
        CyclicBuffer<uint64_t, 3> InteractiveWavesTextureHandles;

        /**
         * @brief Cyclic buffer storing the 2D world positions of the effector in the previous frames.
         */
        CyclicBuffer<glm::vec2, 2> PreviousPositions;

        /**
         * @brief Cyclic buffer storing the physical sizes of the interactive wave fields in the previous frames.
         */
        CyclicBuffer<float, 2> PreviousInteractiveWavesFieldSizes;

        /**
         * @brief The hydrodynamic drag coefficient affecting the resistance the object faces in water.
         */
        float DragCoefficient;

        /**
         * @brief The computed physical world size of the intersection mask projection.
         */
        float IntersectionMaskFieldSize;

        /**
         * @brief The total precalculated volume of the collision mesh, used for buoyancy calculations.
         */
        float MeshTotalVolume;

        /**
         * @brief Multiplier for the amplitude of the wake generated behind the moving effector.
         */
        float BackWaveHeightFactor;

        /**
         * @brief Multiplier for the amplitude of the bow wave generated in front of the moving effector.
         */
        float FrontWaveHeightFactor;

        /**
         * @brief Overall scalar multiplier for the strength/height of all generated dynamic waves.
         */
        float DisplacementStrength;

        /**
         * @brief The resolution (width and height in pixels) of the underlying intersection and wave textures.
         */
        int32_t TexturesResolution;

    public:
        /**
         * @brief Initializes a new water effector component with a precalculated mesh volume.
         * @param Mesh The collision mesh used for the effector.
         * @param Position The initial world position of the effector.
         * @param MeshTotalVolume The precalculated total volume of the provided mesh.
         * @param DragCoefficient The hydrodynamic drag coefficient.
         * @param DisplacementStrength Overall strength multiplier for wave generation.
         * @param BackWaveHeightFactor Amplitude multiplier for trailing waves (wake).
         * @param FrontWaveHeightFactor Amplitude multiplier for leading waves (bow wave).
         * @param TexturesResolution Resolution of the internal simulation textures.
         */
        WaterEffectorComponent(const class Mesh& Mesh,
                               const glm::vec3& Position,
                               float MeshTotalVolume,
                               float DragCoefficient,
                               float DisplacementStrength,
                               float BackWaveHeightFactor,
                               float FrontWaveHeightFactor,
                               int32_t TexturesResolution);

        /**
         * @brief Initializes a new water effector component and automatically calculates the mesh volume.
         * @param Mesh The collision mesh used for the effector (volume will be calculated automatically).
         * @param Position The initial world position of the effector.
         * @param DragCoefficient The hydrodynamic drag coefficient.
         * @param DisplacementStrength Overall strength multiplier for wave generation.
         * @param BackWaveHeightFactor Amplitude multiplier for trailing waves (wake).
         * @param FrontWaveHeightFactor Amplitude multiplier for leading waves (bow wave).
         * @param TexturesResolution Resolution of the internal simulation textures.
         */
        WaterEffectorComponent(const class Mesh& Mesh,
                               const glm::vec3& Position,
                               float DragCoefficient,
                               float DisplacementStrength,
                               float BackWaveHeightFactor,
                               float FrontWaveHeightFactor,
                               int32_t TexturesResolution);

    public:
        /**
         * @brief Cleans up resources associated with the component.
         * Designed to be called when the component is removed from an entity registry.
         * * @param ToDestroy The component instance to be destroyed.
         */
        static void Destroy(const WaterEffectorComponent& ToDestroy);
    };
}
