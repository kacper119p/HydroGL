#pragma once
#include <assimp/scene.h>

#include "Meshes/MeshFacesData.h"
#include "Meshes/MeshVerticesData.h"

namespace Core
{
    class Mesh final
    {
    public:
        static constexpr uint32_t VertexBufferMaxCapacity = 4194304; //Space for 2^22 vertices

    private:
        static MeshVerticesData _vertexBuffer;
        static MeshFacesData _faceBuffer;
        static std::vector<glm::uvec2> _takenVertexMemoryChunks;
        static std::vector<glm::uvec2> _takenFaceMemoryChunks;

        static uint32_t _glVertexArray;
        static uint32_t _glElementBuffer;
        static uint32_t _glVertexBuffer;

    private:
        uint32_t _verticesIndex;
        uint32_t _verticesCount;
        uint32_t _facesIndex;
        uint32_t _facesCount;

    private:
        /**
         * @brief Constructs a new model from a given file.
         * @param FilePath Path to a model file.
         */
        explicit Mesh(const char* FilePath);

    public:
        ~Mesh() = default;

    private:
        static void Initialize();

        static void FreeResources();

        static uint32_t AllocateMemoryChunk(uint32_t VertexCount, std::vector<glm::uvec2>& TakenMemoryChunks);

    private:
        void ProcessScene(const aiScene& Scene);

        void UploadToGpu() const;

    public:
    };
}
