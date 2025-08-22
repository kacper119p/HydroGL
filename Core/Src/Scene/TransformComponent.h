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
        [[nodiscard]] const glm::vec3& GetPosition() const
        {
            return _position;
        }

        void SetPosition(const glm::vec3& Position)
        {
            _position = Position;
            _sceneHierarchyNode->MarkDirty();
        }

        [[nodiscard]] const glm::vec3& GetScale() const
        {
            return _scale;
        }

        void SetScale(const glm::vec3& Scale)
        {
            _scale = Scale;
            _sceneHierarchyNode->MarkDirty();
        }

        [[nodiscard]] const glm::quat& GetRotation() const
        {
            return _rotation;
        }

        void SetRotation(const glm::quat& Rotation)
        {
            _rotation = Rotation;
            _sceneHierarchyNode->MarkDirty();
        }

        void SetEulerAngles(const glm::vec3& EulerAngles)
        {
            _rotation = glm::quat(glm::radians(EulerAngles));
            _sceneHierarchyNode->MarkDirty();
        }

        [[nodiscard]] glm::vec3 GetEulerAngles() const
        {
            return glm::degrees(glm::eulerAngles(_rotation));
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
