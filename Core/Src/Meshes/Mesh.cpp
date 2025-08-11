#include "Mesh.h"

#include <Assimp/Importer.hpp>
#include <Assimp/postprocess.h>
#include <glad/glad.h>

#include "Debug.h"

static_assert(sizeof(ai_real) == sizeof(float),
              "ai_real has to be 32 bit float! ASSIMP_DOUBLE_PRECISION=OFF is required!");

namespace Core
{
    MeshVerticesData Mesh::_vertexBuffer(VertexBufferMaxCapacity);
    MeshFacesData Mesh::_faceBuffer(VertexBufferMaxCapacity);
    std::vector<glm::uvec2> Mesh::_takenVertexMemoryChunks(4096);
    std::vector<glm::uvec2> Mesh::_takenFaceMemoryChunks(4096);

    uint32_t Mesh::_glVertexArray = 0;
    uint32_t Mesh::_glElementBuffer = 0;
    uint32_t Mesh::_glVertexBuffer = 0;

    Mesh::Mesh(const char* FilePath)
    {
        Assimp::Importer importer = Assimp::Importer();
        importer.SetPropertyInteger(
            AI_CONFIG_PP_SBP_REMOVE,
            aiPrimitiveType_LINE | aiPrimitiveType_POINT
        );

        const aiScene* scene = importer.ReadFile(FilePath,
                                                 aiProcess_Triangulate | aiProcess_GenNormals);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
            !scene->mRootNode)
        {
            return;
        }

        ProcessScene(*scene);
        UploadToGpu();
    }

    void Mesh::Initialize()
    {
        glGenVertexArrays(1, &_glVertexArray);
        glGenBuffers(1, &_glElementBuffer);
        glGenBuffers(1, &_glVertexBuffer);

        glBindVertexArray(_glVertexArray);
        glBindBuffer(GL_ARRAY_BUFFER, _glVertexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _glElementBuffer);

        glBufferData(GL_ELEMENT_ARRAY_BUFFER, VertexBufferMaxCapacity * sizeof(unsigned int), nullptr, GL_STATIC_DRAW);
        glBufferData(GL_ARRAY_BUFFER, VertexBufferMaxCapacity * MeshVerticesData::VertexSize, nullptr, GL_STATIC_DRAW);

        std::byte* base = reinterpret_cast<std::byte*>(_vertexBuffer.Positions);

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

    void Mesh::FreeResources()
    {
        glDeleteVertexArrays(1, &_glVertexArray);
        glDeleteBuffers(1, &_glElementBuffer);
        glDeleteBuffers(1, &_glVertexBuffer);

        //TODO: Free remaining resources
    }

    void Mesh::ProcessScene(const aiScene& Scene)
    {
        for (uint32_t i = 0; i < Scene.mNumMeshes; ++i)
        {
            _verticesCount += Scene.mMeshes[i]->mNumVertices;
            _facesCount += Scene.mMeshes[i]->mNumFaces;
        }
        _facesCount *= 3;

        _verticesIndex = AllocateMemoryChunk(_verticesCount, _takenVertexMemoryChunks);

        uint32_t offset = _verticesIndex;
        for (uint32_t currentMesh = 0; currentMesh < Scene.mNumMeshes; ++currentMesh)
        {
            const aiMesh* mesh = Scene.mMeshes[currentMesh];

            memcpy(_vertexBuffer.Positions + offset, mesh->mVertices, mesh->mNumVertices * sizeof(glm::vec3));
            memcpy(_vertexBuffer.TexCoords + offset, mesh->mTextureCoords[0], mesh->mNumVertices * sizeof(glm::vec2));
            memcpy(_vertexBuffer.Normals + offset, mesh->mNormals, mesh->mNumVertices * sizeof(glm::vec3));
            memcpy(_vertexBuffer.Tangents + offset, mesh->mTangents, mesh->mNumVertices * sizeof(glm::vec3));

            offset += mesh->mNumVertices;
        }

        _facesIndex = AllocateMemoryChunk(_facesCount, _takenFaceMemoryChunks);

        offset = _facesIndex;
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

    void Mesh::UploadToGpu() const
    {
        glBindVertexArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, _glVertexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _glElementBuffer);

        const std::byte* base = reinterpret_cast<std::byte*>(_vertexBuffer.Positions);

        glBufferSubData(GL_ARRAY_BUFFER,
                        static_cast<GLintptr>(_verticesIndex * sizeof(glm::vec3)),
                        static_cast<GLintptr>(_verticesCount * sizeof(glm::vec3)),
                        _vertexBuffer.Positions + _verticesIndex);
        glBufferSubData(GL_ARRAY_BUFFER,
                        static_cast<GLintptr>((reinterpret_cast<std::byte*>(_vertexBuffer.TexCoords) - base)
                                              + _verticesIndex * sizeof(glm::vec2)),
                        static_cast<GLintptr>(_verticesCount * sizeof(glm::vec2)),
                        _vertexBuffer.TexCoords + _verticesIndex);
        glBufferSubData(GL_ARRAY_BUFFER,
                        static_cast<GLintptr>((reinterpret_cast<std::byte*>(_vertexBuffer.Normals) - base)
                                              + _verticesIndex * sizeof(glm::vec3)),
                        static_cast<GLintptr>(_verticesCount * sizeof(glm::vec3)),
                        _vertexBuffer.Normals + _verticesIndex);
        glBufferSubData(GL_ARRAY_BUFFER,
                        static_cast<GLintptr>((reinterpret_cast<std::byte*>(_vertexBuffer.Tangents) - base)
                                              + _verticesIndex * sizeof(glm::vec3)),
                        static_cast<GLintptr>(_verticesCount * sizeof(glm::vec3)),
                        _vertexBuffer.Tangents + _verticesIndex);

        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER,
                        static_cast<GLintptr>(_facesIndex * sizeof(uint32_t) * 3),
                        static_cast<GLintptr>(_facesCount * sizeof(uint32_t)),
                        _faceBuffer.Indices + _verticesIndex);
    }

    uint32_t Mesh::AllocateMemoryChunk(uint32_t VertexCount, std::vector<glm::uvec2>& TakenMemoryChunks)
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
}
