#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include "Meshes/Mesh.h"
#include "Meshes/MeshFacesData.h"
#include "Meshes/MeshVerticesData.h"
#include "glad/glad.h"


namespace Core
{
    class MeshManager
    {
    public:
        static constexpr uint32_t VertexBufferMaxCapacity = 4194304; //Space for 2^22 vertices

    private:
        static MeshVerticesData _vertexBuffer;
        static MeshFacesData _faceBuffer;
        static std::vector<glm::uvec2> _takenVertexMemoryChunks;
        static std::vector<glm::uvec2> _takenFaceMemoryChunks;

        static std::unordered_map<std::string, Mesh> _meshCache;

        static uint32_t _glVertexArray;
        static uint32_t _glElementBuffer;
        static uint32_t _glVertexBuffer;

    public:
        static void Initialize();

        static void FreeResources();

        [[nodiscard]] static Mesh GetMesh(const std::string& Path);

        static bool DeleteMesh(const std::string& Path);

        static bool DeleteMesh(const Mesh& Mesh);

        static void DeleteAllMeshes();

        [[nodiscard]] static bool IsValid(const std::string& Path);

        [[nodiscard]] static bool IsValid(const Mesh& Mesh);

        [[nodiscard]] static std::string GetMeshPath(const Mesh& Mesh);

        static uint32_t AllocateMemoryChunk(uint32_t VertexCount, std::vector<glm::uvec2>& TakenMemoryChunks);

        static void BindVao()
        {
            glBindVertexArray(_glVertexArray);
        }

    private:
        static Mesh LoadMesh(const std::string& Path);

        static void ProcessScene(const aiScene& Scene, Mesh& Mesh);

        static void UploadToGpu(const Mesh& Mesh);
    };
}
