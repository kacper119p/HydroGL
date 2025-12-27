#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include "Meshes/Mesh.h"
#include "Meshes/MeshFacesData.h"
#include "Meshes/MeshVerticesData.h"
#include "glad/glad.h"
#include "Utility/GlResourceUnique.h"


namespace Core
{
    class MeshManager
    {
    public:
        static constexpr uint32_t VertexBufferMaxCapacity = 4194304; //Space for 2^22 vertices
    private:
        struct MemoryChunk
        {
            uint32_t Start;
            uint32_t End;

            /**
             * @brief Single parameter constructor used for searching.
             * @param Start The start index of the memory chunk.
             */
            explicit MemoryChunk(const uint32_t Start) : Start(Start), End(0)
            {
            }

            /**
             * @brief Constructor.
             * @param Start The start index of the memory chunk.
             * @param End The end index of the memory chunk.
             */
            MemoryChunk(const uint32_t Start, const uint32_t End) : Start(Start), End(End)
            {
            }

            friend bool operator==(const MemoryChunk& Lhs, const MemoryChunk& Rhs)
            {
                return Lhs.Start == Rhs.Start;
            }

            friend bool operator!=(const MemoryChunk& Lhs, const MemoryChunk& Rhs)
            {
                return !(Lhs == Rhs);
            }
        };

    private:
        static MeshVerticesData _vertexBuffer;
        static MeshFacesData _faceBuffer;
        static std::vector<MemoryChunk> _takenVertexMemoryChunks;
        static std::vector<MemoryChunk> _takenFaceMemoryChunks;

        static std::unordered_map<std::string, Mesh> _meshCache;

        static GlVertexArrayUnique _glVertexArray;
        static GlBufferUnique _glElementBuffer;
        static GlBufferUnique _glVertexBuffer;

    public:
        [[nodiscard]] static uint32_t GetVertexBufferId()
        {
            return _glVertexBuffer.Get();
        }

        [[nodiscard]] static uint32_t GetElementBufferId()
        {
            return _glElementBuffer.Get();
        }

        [[nodiscard]] static glm::vec3* GetPositionsBufferData(const uint32_t VertexIndex)
        {
            return _vertexBuffer.Positions + VertexIndex;
        }

        [[nodiscard]] static glm::vec2* GetTexCoordsBufferData(const uint32_t VertexIndex)
        {
            return _vertexBuffer.TexCoords + VertexIndex;
        }

        [[nodiscard]] static glm::vec3* GetNormalsBufferData(const uint32_t VertexIndex)
        {
            return _vertexBuffer.Normals + VertexIndex;
        }

        [[nodiscard]] static glm::vec3* GetTangentsBufferData(const uint32_t VertexIndex)
        {
            return _vertexBuffer.Tangents + VertexIndex;
        }

        [[nodiscard]] static uint32_t* GetIndicesBufferData(const uint32_t IndexIndex)
        {
            return _faceBuffer.Indices + IndexIndex;
        }

        static void Initialize();

        [[nodiscard]] static Mesh GetMesh(const std::string& Path);

        static bool DeleteMesh(const std::string& Path);

        static bool DeleteMesh(const Mesh& Mesh);

        static void DeleteAllMeshes();

        [[nodiscard]] static bool IsValid(const std::string& Path);

        [[nodiscard]] static bool IsValid(const Mesh& Mesh);

        [[nodiscard]] static std::string GetMeshPath(const Mesh& Mesh);

        static uint32_t AllocateMemoryChunk(uint32_t VertexCount, std::vector<MemoryChunk>& TakenMemoryChunks);

        static void BindVao()
        {
            glBindVertexArray(_glVertexArray.Get());
        }

    private:
        static Mesh LoadMesh(const std::string& Path);

        static void ProcessScene(const aiScene& Scene, Mesh& Mesh);

        static void UploadToGpu(const Mesh& Mesh);

        static void FreeMeshMemoryChunk(const Mesh& Mesh);
    };
}
