#include "MeshManager.h"

#include <ranges>
#include <Assimp/Importer.hpp>
#include <Assimp/postprocess.h>

#include "Debug.h"

namespace Core
{
    static_assert(sizeof(ai_real) == sizeof(float),
                  "ai_real has to be 32 bit float! ASSIMP_DOUBLE_PRECISION=OFF is required!");

    MeshVerticesData MeshManager::_vertexBuffer(VertexBufferMaxCapacity);
    MeshFacesData MeshManager::_faceBuffer(VertexBufferMaxCapacity);

    std::vector<MeshManager::MemoryChunk> MeshManager::_takenVertexMemoryChunks;

    std::vector<MeshManager::MemoryChunk> MeshManager::_takenFaceMemoryChunks;

    std::unordered_map<std::string, Mesh> MeshManager::_meshCache(256);

    GlVertexArrayUnique MeshManager::_glVertexArray;
    GlBufferUnique MeshManager::_glElementBuffer;
    GlBufferUnique MeshManager::_glVertexBuffer;

    void MeshManager::Initialize()
    {
        _glVertexArray = GlVertexArrayUnique::Create();
        _glElementBuffer = GlBufferUnique::Create();
        _glVertexBuffer = GlBufferUnique::Create();

        glNamedBufferStorage(_glElementBuffer, VertexBufferMaxCapacity * sizeof(uint32_t),
                             nullptr, GL_DYNAMIC_STORAGE_BIT);
        glNamedBufferStorage(_glVertexBuffer, VertexBufferMaxCapacity * MeshVerticesData::VertexSize,
                             nullptr, GL_DYNAMIC_STORAGE_BIT);

        // Face indices
        glVertexArrayElementBuffer(_glVertexArray, _glElementBuffer);

        const std::byte* base = reinterpret_cast<std::byte*>(_vertexBuffer.Positions);

        //Positions
        glEnableVertexArrayAttrib(_glVertexArray, 0);
        glVertexArrayAttribFormat(_glVertexArray, 0, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(_glVertexArray, 0, 0);
        glVertexArrayVertexBuffer(_glVertexArray, 0, _glVertexBuffer, 0, sizeof(glm::vec3));

        //TexCoords
        const ptrdiff_t texCoordsOffset = reinterpret_cast<std::byte*>(_vertexBuffer.TexCoords) - base;
        glEnableVertexArrayAttrib(_glVertexArray, 1);
        glVertexArrayAttribFormat(_glVertexArray, 1, 2, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(_glVertexArray, 1, 1);
        glVertexArrayVertexBuffer(_glVertexArray, 1, _glVertexBuffer, texCoordsOffset, sizeof(glm::vec2));

        //Normals
        const ptrdiff_t normalsOffset = reinterpret_cast<std::byte*>(_vertexBuffer.Normals) - base;
        glEnableVertexArrayAttrib(_glVertexArray, 2);
        glVertexArrayAttribFormat(_glVertexArray, 2, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(_glVertexArray, 2, 2);
        glVertexArrayVertexBuffer(_glVertexArray, 2, _glVertexBuffer, normalsOffset, sizeof(glm::vec3));

        //Tangents
        const ptrdiff_t tangentsOffset = reinterpret_cast<std::byte*>(_vertexBuffer.Tangents) - base;
        glEnableVertexArrayAttrib(_glVertexArray, 3);
        glVertexArrayAttribFormat(_glVertexArray, 3, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(_glVertexArray, 3, 3);
        glVertexArrayVertexBuffer(_glVertexArray, 3, _glVertexBuffer, tangentsOffset, sizeof(glm::vec3));
    }

    Mesh MeshManager::GetMesh(const std::string& Path)
    {
        if (const auto iterator = _meshCache.find(Path); iterator != _meshCache.end())
        {
            return iterator->second;
        }

        Mesh newMesh = LoadMesh(Path);
        _meshCache.emplace(Path, newMesh);
        return newMesh;
    }

    bool MeshManager::DeleteMesh(const std::string& Path)
    {
        const auto iterator = _meshCache.find(Path);
        if (iterator != _meshCache.end())
        {
            FreeMeshMemoryChunk(iterator->second);
            _meshCache.erase(iterator);
            return true;
        }
        return false;
    }

    bool MeshManager::DeleteMesh(const Mesh& Mesh)
    {
        for (const auto& [path, mesh] : _meshCache)
            if (mesh == Mesh)
            {
                FreeMeshMemoryChunk(Mesh);
                _meshCache.erase(path);
                return true;
            }
        return false;
    }

    void MeshManager::DeleteAllMeshes()
    {
        _takenVertexMemoryChunks.clear();
        _takenFaceMemoryChunks.clear();
        _meshCache.clear();
    }

    bool MeshManager::IsValid(const std::string& Path)
    {
        return _meshCache.contains(Path);
    }

    bool MeshManager::IsValid(const Mesh& Mesh)
    {
        for (const Core::Mesh& mesh : _meshCache | std::views::values)
        {
            if (mesh == Mesh)
            {
                return true;
            }
        }
        return false;
    }

    std::string MeshManager::GetMeshPath(const Mesh& Mesh)
    {
        for (const auto& [path, mesh] : _meshCache)
        {
            if (mesh == Mesh)
            {
                return path;
            }
        }
        return "";
    }

    void MeshManager::ProcessScene(const aiScene& Scene, Mesh& Mesh)
    {
        for (uint32_t i = 0; i < Scene.mNumMeshes; ++i)
        {
            Mesh._verticesCount += Scene.mMeshes[i]->mNumVertices;
            Mesh._indicesCount += Scene.mMeshes[i]->mNumFaces;
        }
        Mesh._indicesCount *= 3;

        Mesh._verticesIndex = AllocateMemoryChunk(Mesh._verticesCount, _takenVertexMemoryChunks);

        uint32_t offset = Mesh._verticesIndex;
        for (uint32_t currentMesh = 0; currentMesh < Scene.mNumMeshes; ++currentMesh)
        {
            const aiMesh* mesh = Scene.mMeshes[currentMesh];

            memcpy(_vertexBuffer.Positions + offset, mesh->mVertices, mesh->mNumVertices * sizeof(glm::vec3));
            memcpy(_vertexBuffer.Normals + offset, mesh->mNormals, mesh->mNumVertices * sizeof(glm::vec3));
            memcpy(_vertexBuffer.Tangents + offset, mesh->mTangents, mesh->mNumVertices * sizeof(glm::vec3));

            const aiVector3D* texCoords = mesh->mTextureCoords[0];
            for (uint32_t i = 0; i < mesh->mNumVertices; ++i)
            {
                _vertexBuffer.TexCoords[offset + i] = glm::vec2(texCoords[i].x, texCoords[i].y);
            }

            offset += mesh->mNumVertices;
        }

        Mesh._indicesIndex = AllocateMemoryChunk(Mesh._indicesCount, _takenFaceMemoryChunks);

        offset = Mesh._indicesIndex;
        for (uint32_t currentMesh = 0; currentMesh < Scene.mNumMeshes; ++currentMesh)
        {
            const aiMesh* mesh = Scene.mMeshes[currentMesh];

            for (uint32_t i = 0; i < mesh->mNumFaces; ++i)
            {
                const aiFace& face = mesh->mFaces[i];
                CHECK(face.mNumIndices == 3, "Mesh is not triangulated!");
                for (uint32_t j = 0; j < 3; j++)
                {
                    _faceBuffer.Indices[offset] = face.mIndices[j];
                    offset++;
                }
            }
        }

        Mesh._boundingBox = BoundingBox(_vertexBuffer.Positions + Mesh._verticesIndex, Mesh._verticesCount);
    }

    void MeshManager::UploadToGpu(const Mesh& Mesh)
    {
        const std::byte* base = reinterpret_cast<std::byte*>(_vertexBuffer.Positions);

        glNamedBufferSubData(_glVertexBuffer,
                             static_cast<GLintptr>(Mesh._verticesIndex * sizeof(glm::vec3)),
                             static_cast<GLintptr>(Mesh._verticesCount * sizeof(glm::vec3)),
                             _vertexBuffer.Positions + Mesh._verticesIndex);
        glNamedBufferSubData(_glVertexBuffer,
                             static_cast<GLintptr>((reinterpret_cast<std::byte*>(_vertexBuffer.TexCoords) - base)
                                                   + Mesh._verticesIndex * sizeof(glm::vec2)),
                             static_cast<GLintptr>(Mesh._verticesCount * sizeof(glm::vec2)),
                             _vertexBuffer.TexCoords + Mesh._verticesIndex);
        glNamedBufferSubData(_glVertexBuffer,
                             static_cast<GLintptr>((reinterpret_cast<std::byte*>(_vertexBuffer.Normals) - base)
                                                   + Mesh._verticesIndex * sizeof(glm::vec3)),
                             static_cast<GLintptr>(Mesh._verticesCount * sizeof(glm::vec3)),
                             _vertexBuffer.Normals + Mesh._verticesIndex);
        glNamedBufferSubData(_glVertexBuffer,
                             static_cast<GLintptr>((reinterpret_cast<std::byte*>(_vertexBuffer.Tangents) - base)
                                                   + Mesh._verticesIndex * sizeof(glm::vec3)),
                             static_cast<GLintptr>(Mesh._verticesCount * sizeof(glm::vec3)),
                             _vertexBuffer.Tangents + Mesh._verticesIndex);

        glNamedBufferSubData(_glElementBuffer,
                             static_cast<GLintptr>(Mesh._indicesIndex * sizeof(uint32_t)),
                             static_cast<GLintptr>(Mesh._indicesCount * sizeof(uint32_t)),
                             _faceBuffer.Indices + Mesh._indicesIndex);
    }

    void MeshManager::FreeMeshMemoryChunk(const Mesh& Mesh)
    {
        const MemoryChunk vertexChunkToFree(Mesh._verticesIndex);
        const MemoryChunk faceChunkToFree(Mesh._indicesIndex);

        std::erase(_takenVertexMemoryChunks, vertexChunkToFree);
        std::erase(_takenFaceMemoryChunks, faceChunkToFree);
    }

    uint32_t MeshManager::AllocateMemoryChunk(const uint32_t VertexCount, std::vector<MemoryChunk>& TakenMemoryChunks)
    {
        if (TakenMemoryChunks.empty())
        {
            if (VertexCount <= VertexBufferMaxCapacity)
            {
                TakenMemoryChunks.emplace_back(0, VertexCount - 1);
                return 0;
            }
            CHECK(false, "Program ran out of allocated vertex memory!");
            return std::numeric_limits<uint32_t>::max();
        }

        if (TakenMemoryChunks[0].Start >= VertexCount)
        {
            TakenMemoryChunks.emplace(TakenMemoryChunks.begin(), 0, VertexCount - 1);
            return 0;
        }

        for (uint32_t i = 0; i < TakenMemoryChunks.size() - 1; ++i)
        {
            const uint32_t areaLowerBound = TakenMemoryChunks[i].End;
            const uint32_t areaUpperBound = TakenMemoryChunks[i + 1].Start;
            if (areaUpperBound - areaLowerBound > VertexCount)
            {
                TakenMemoryChunks.emplace(TakenMemoryChunks.begin() + i + 1, areaLowerBound + 1,
                                          areaLowerBound + VertexCount);
                return areaLowerBound + 1;
            }
        }

        const uint32_t lastChunkEnd = TakenMemoryChunks.back().End;
        if (VertexBufferMaxCapacity > lastChunkEnd + 1
            && VertexBufferMaxCapacity - 1 - lastChunkEnd >= VertexCount)
        {
            const uint32_t newStart = lastChunkEnd + 1;
            TakenMemoryChunks.emplace_back(newStart, newStart + VertexCount - 1);
            return newStart;
        }

        CHECK(false, "Program ran out of allocated vertex memory!");
        return std::numeric_limits<uint32_t>::max();
    }

    Mesh MeshManager::LoadMesh(const std::string& Path)
    {
        Assimp::Importer importer = Assimp::Importer();
        importer.SetPropertyInteger(
            AI_CONFIG_PP_SBP_REMOVE,
            aiPrimitiveType_LINE | aiPrimitiveType_POINT
        );

        const aiScene* scene = importer.ReadFile(Path,
                                                 aiProcess_Triangulate | aiProcess_GenNormals);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
            !scene->mRootNode)
        {
            return Mesh();
        }

        Mesh newMesh;
        ProcessScene(*scene, newMesh);
        UploadToGpu(newMesh);
        return newMesh;
    }
}
