#pragma once

#include <entt/entity/registry.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "SceneNode.h"

namespace Core
{
    class TransformComponent
    {
        friend class SceneNode;

    private:
        glm::vec3 _position;
        glm::vec3 _scale;
        glm::quat _rotation;
        glm::mat4 _localToWorldMatrix = glm::identity<glm::mat4>();
        glm::mat4 _localToParentMatrix = glm::identity<glm::mat4>();
        SceneNode* _sceneHierarchyNode = nullptr;
        bool _isDirty = false;

    public:
        static void on_construct(entt::registry& Registry, entt::entity Id);
        static void on_destruct(entt::registry& Registry, entt::entity Id);

    public:
        [[nodiscard]] const glm::vec3& GetPosition() const { return _position; }

        void SetPosition(const glm::vec3& Position)
        {
            _position = Position;
            _isDirty = true;
        }

        [[nodiscard]] const glm::vec3& GetScale() const { return _scale; }

        void SetScale(const glm::vec3& Scale)
        {
            _scale = Scale;
            _isDirty = true;
        }

        [[nodiscard]] const glm::quat& GetRotation() const { return _rotation; }

        void SetRotation(const glm::quat& Rotation)
        {
            _rotation = Rotation;
            _isDirty = true;
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
    };
} // namespace Core
