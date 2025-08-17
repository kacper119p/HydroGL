#include "MeshManager.h"

#include <ranges>
#include <Assimp/Importer.hpp>
#include <Assimp/postprocess.h>

#include "Debug.h"
#include "glad/glad.h"

namespace Core
{
    static_assert(sizeof(ai_real) == sizeof(float),
                  "ai_real has to be 32 bit float! ASSIMP_DOUBLE_PRECISION=OFF is required!");

    MeshVerticesData MeshManager::_vertexBuffer(VertexBufferMaxCapacity);
    MeshFacesData MeshManager::_faceBuffer(VertexBufferMaxCapacity);
    std::vector<glm::uvec2> MeshManager::_takenVertexMemoryChunks(4096);
    std::vector<glm::uvec2> MeshManager::_takenFaceMemoryChunks(4096);

    std::unordered_map<std::string, Mesh> MeshManager::_meshCache(256);

    uint32_t MeshManager::_glVertexArray = 0;
    uint32_t MeshManager::_glElementBuffer = 0;
    uint32_t MeshManager::_glVertexBuffer = 0;

    void MeshManager::Initialize()
    {
        glGenVertexArrays(1, &_glVertexArray);
        glGenBuffers(1, &_glElementBuffer);
        glGenBuffers(1, &_glVertexBuffer);

        glBindVertexArray(_glVertexArray);
        glBindBuffer(GL_ARRAY_BUFFER, _glVertexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _glElementBuffer);

        glBufferData(GL_ELEMENT_ARRAY_BUFFER, VertexBufferMaxCapacity * sizeof(unsigned int), nullptr, GL_STATIC_DRAW);
        glBufferData(GL_ARRAY_BUFFER, VertexBufferMaxCapacity * MeshVerticesData::VertexSize, nullptr, GL_STATIC_DRAW);

        const std::byte* base = reinterpret_cast<std::byte*>(_vertexBuffer.Positions);

        //Vertex::Position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3),
                              reinterpret_cast<void*>(0));

        //Vertex::TexCoords
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2),
                              reinterpret_cast<void*>(reinterpret_cast<std::byte*>(_vertexBuffer.TexCoords) - base));

        //Vertex::Normal
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3),
                              reinterpret_cast<void*>(reinterpret_cast<std::byte*>(_vertexBuffer.Normals) - base));

        //Vertex::Tangent
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3),
                              reinterpret_cast<void*>(reinterpret_cast<std::byte*>(_vertexBuffer.Tangents) - base));

        glBindVertexArray(0);
    }

    void MeshManager::FreeResources()
    {
        glDeleteVertexArrays(1, &_glVertexArray);
        glDeleteBuffers(1, &_glElementBuffer);
        glDeleteBuffers(1, &_glVertexBuffer);

        //TODO: Free remaining resources
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
            //TODO: Free block
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
                //TODO: Free block
                _meshCache.erase(path);
                return true;
            }
        return false;
    }

    void MeshManager::DeleteAllMeshes()
    {
        for (const auto& pair : _meshCache)
        {
            //TODO: Free block
        }
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
            Mesh._facesCount += Scene.mMeshes[i]->mNumFaces;
        }
        Mesh._facesCount *= 3;

        Mesh._verticesIndex = AllocateMemoryChunk(Mesh._verticesCount, _takenVertexMemoryChunks);

        uint32_t offset = Mesh._verticesIndex;
        for (uint32_t currentMesh = 0; currentMesh < Scene.mNumMeshes; ++currentMesh)
        {
            const aiMesh* mesh = Scene.mMeshes[currentMesh];

            memcpy(_vertexBuffer.Positions + offset, mesh->mVertices, mesh->mNumVertices * sizeof(glm::vec3));
            memcpy(_vertexBuffer.TexCoords + offset, mesh->mTextureCoords[0], mesh->mNumVertices * sizeof(glm::vec2));
            memcpy(_vertexBuffer.Normals + offset, mesh->mNormals, mesh->mNumVertices * sizeof(glm::vec3));
            memcpy(_vertexBuffer.Tangents + offset, mesh->mTangents, mesh->mNumVertices * sizeof(glm::vec3));

            offset += mesh->mNumVertices;
        }

        Mesh._facesIndex = AllocateMemoryChunk(Mesh._facesCount, _takenFaceMemoryChunks);

        offset = Mesh._facesIndex;
        for (uint32_t currentMesh = 0; currentMesh < Scene.mNumMeshes; ++currentMesh)
        {
            const aiMesh* mesh = Scene.mMeshes[currentMesh];

            for (uint32_t i = 0; i < mesh->mNumFaces; ++i)
            {
                const aiFace& face = mesh->mFaces[i];
                CHECK_MESSAGE(face.mNumIndices == 3, "Mesh is not triangulated!");
                for (unsigned int j = 0; j < 3; j++)
                {
                    _faceBuffer.Indices[offset] = face.mIndices[0];
                    offset++;
                }
            }
        }
    }

    void MeshManager::UploadToGpu(const Mesh& Mesh)
    {
        glBindVertexArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, _glVertexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _glElementBuffer);

        const std::byte* base = reinterpret_cast<std::byte*>(_vertexBuffer.Positions);

        glBufferSubData(GL_ARRAY_BUFFER,
                        static_cast<GLintptr>(Mesh._verticesIndex * sizeof(glm::vec3)),
                        static_cast<GLintptr>(Mesh._verticesCount * sizeof(glm::vec3)),
                        _vertexBuffer.Positions + Mesh._verticesIndex);
        glBufferSubData(GL_ARRAY_BUFFER,
                        static_cast<GLintptr>((reinterpret_cast<std::byte*>(_vertexBuffer.TexCoords) - base)
                                              + Mesh._verticesIndex * sizeof(glm::vec2)),
                        static_cast<GLintptr>(Mesh._verticesCount * sizeof(glm::vec2)),
                        _vertexBuffer.TexCoords + Mesh._verticesIndex);
        glBufferSubData(GL_ARRAY_BUFFER,
                        static_cast<GLintptr>((reinterpret_cast<std::byte*>(_vertexBuffer.Normals) - base)
                                              + Mesh._verticesIndex * sizeof(glm::vec3)),
                        static_cast<GLintptr>(Mesh._verticesCount * sizeof(glm::vec3)),
                        _vertexBuffer.Normals + Mesh._verticesIndex);
        glBufferSubData(GL_ARRAY_BUFFER,
                        static_cast<GLintptr>((reinterpret_cast<std::byte*>(_vertexBuffer.Tangents) - base)
                                              + Mesh._verticesIndex * sizeof(glm::vec3)),
                        static_cast<GLintptr>(Mesh._verticesCount * sizeof(glm::vec3)),
                        _vertexBuffer.Tangents + Mesh._verticesIndex);

        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER,
                        static_cast<GLintptr>(Mesh._facesIndex * sizeof(uint32_t) * 3),
                        static_cast<GLintptr>(Mesh._facesCount * sizeof(uint32_t)),
                        _faceBuffer.Indices + Mesh._verticesIndex);
    }

    uint32_t MeshManager::AllocateMemoryChunk(uint32_t VertexCount, std::vector<glm::uvec2>& TakenMemoryChunks)
    {
        if (TakenMemoryChunks.empty())
        {
            TakenMemoryChunks.emplace_back(0, VertexCount);
            return 0;
        }

        if (TakenMemoryChunks.size() == 1)
        {
            const uint32_t areaStart = TakenMemoryChunks[0].y + 1;
            TakenMemoryChunks.emplace_back(areaStart, areaStart + VertexCount);
            return 0;
        }

        for (int i = 0; i < TakenMemoryChunks.size() - 1; ++i)
        {
            const uint32_t areaLowerBound = TakenMemoryChunks[i].y;
            const uint32_t areaUpperBound = TakenMemoryChunks[i + 1].x;
            if (areaUpperBound - areaLowerBound > VertexCount)
            {
                TakenMemoryChunks.emplace(TakenMemoryChunks.begin() + i + 1, areaLowerBound + 1, areaUpperBound - 1);
                return areaLowerBound + 1;
            }
        }

        CHECK_MESSAGE(false, "Program ran out of allocated vertex memory!")
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
