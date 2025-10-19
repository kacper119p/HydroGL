#pragma once

#include <entt/entity/registry.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "SceneNode.h"

namespace Core
{
    struct TransformComponent
    {
        friend class SceneNode;

    private:
        glm::vec3 _position;
        glm::vec3 _scale = glm::vec3(1.0f);
        glm::quat _rotation = glm::identity<glm::quat>();
        glm::mat4 _localToWorldMatrix = glm::identity<glm::mat4>();
        glm::mat4 _localToParentMatrix = glm::identity<glm::mat4>();
        SceneNode* _sceneHierarchyNode = nullptr;
        bool _isDirty = false;

    public:
        static void on_construct(entt::registry& Registry, entt::entity Id);

        static void on_destroy(entt::registry& Registry, entt::entity Id);

    public:
        [[nodiscard]] const glm::vec3& GetLocalPosition() const
        {
            return _position;
        }

        void SetLocalPosition(const glm::vec3& Position)
        {
            _position = Position;
            _sceneHierarchyNode->MarkDirty();
        }

        [[nodiscard]] const glm::vec3& GetLocalScale() const
        {
            return _scale;
        }

        void SetLocalScale(const glm::vec3& Scale)
        {
            _scale = Scale;
            _sceneHierarchyNode->MarkDirty();
        }

        [[nodiscard]] const glm::quat& GetLocalRotation() const
        {
            if (_isDirty)
            {
                _sceneHierarchyNode->UpdateMatrices();
            }
            return _rotation;
        }

        [[nodiscard]] glm::quat GetWorldRotation() const
        {
            if (_isDirty)
            {
                _sceneHierarchyNode->UpdateMatrices();
            }
            return glm::quat_cast(_localToWorldMatrix);
        }

        void SetLocalRotation(const glm::quat& Rotation)
        {
            _rotation = Rotation;
            _sceneHierarchyNode->MarkDirty();
        }

        void SetLocalEulerAngles(const glm::vec3& EulerAngles)
        {
            _rotation = glm::quat(glm::radians(EulerAngles));
            _sceneHierarchyNode->MarkDirty();
        }

        [[nodiscard]] glm::vec3 GetWorldPosition() const
        {
            if (_isDirty)
            {
                _sceneHierarchyNode->UpdateMatrices();
            }
            return glm::vec3(GetLocalToWorldMatrix()[3]);
        }


        void SetWorldPosition(const glm::vec3& Position)
        {
            if (_sceneHierarchyNode->GetParent())
            {
                const SceneNode* parentNode = _sceneHierarchyNode->GetParent();
                const glm::mat4& parentWorld = parentNode->GetTransformComponent().GetLocalToWorldMatrix();
                const glm::mat4 parentWorldInv = glm::inverse(parentWorld);
                _position = glm::vec3(parentWorldInv * glm::vec4(Position, 1.0f));
            } else
            {
                _position = Position;
            }
            _sceneHierarchyNode->MarkDirty();
        }

        [[nodiscard]] glm::vec3 GetLocalEulerAngles() const
        {
            return glm::degrees(glm::eulerAngles(_rotation));
        }

        [[nodiscard]] glm::vec3 GetWorldEulerAngles() const
        {
            if (_isDirty)
            {
                _sceneHierarchyNode->UpdateMatrices();
            }
            return glm::degrees(glm::eulerAngles(GetWorldRotation()));
        }

        [[nodiscard]] glm::vec3 GetWorldRight() const
        {
            if (_isDirty)
            {
                _sceneHierarchyNode->UpdateMatrices();
            }
            return glm::normalize(_localToWorldMatrix[0]);
        }

        [[nodiscard]] glm::vec3 GetWorldUp() const
        {
            if (_isDirty)
            {
                _sceneHierarchyNode->UpdateMatrices();
            }
            return glm::normalize(_localToWorldMatrix[1]);
        }

        [[nodiscard]] glm::vec3 GetWorldForward() const
        {
            if (_isDirty)
            {
                _sceneHierarchyNode->UpdateMatrices();
            }
            return -glm::normalize(_localToWorldMatrix[2]);
        }

        [[nodiscard]] const glm::mat4& GetLocalToWorldMatrix() const
        {
            if (_isDirty)
            {
                _sceneHierarchyNode->UpdateMatrices();
            }
            return _localToWorldMatrix;
        }

        [[nodiscard]] const glm::mat4& GetLocalToParentMatrix() const
        {
            if (_isDirty)
            {
                _sceneHierarchyNode->UpdateMatrices();
            }
            return _localToParentMatrix;
        }

        [[nodiscard]] SceneNode& GetSceneHierarchyNode() const
        {
            return *_sceneHierarchyNode;
        }
    };
}
