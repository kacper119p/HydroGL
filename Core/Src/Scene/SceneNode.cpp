#include "SceneNode.h"

#include <glm/gtx/quaternion.hpp>

#include "TransformComponent.h"

namespace Core
{
    boost::object_pool<SceneNode, boost::default_user_allocator_new_delete> SceneNode::_memoryPool;

    SceneNode::~SceneNode()
    {
        for (int i = 0; i < _children.size(); ++i)
        {
            const SceneNode* child = _children[i];
            child->_entityRegistry->destroy(child->_entityId);
        }
    }

    SceneNode* SceneNode::Create(entt::registry* EntityRegistry, const entt::entity EntityId)
    {
        return _memoryPool.construct(EntityRegistry, EntityId);
    }

    void SceneNode::MarkDirty() const
    {
        TransformComponent& transform = _entityRegistry->get<TransformComponent>(_entityId);
        transform._isDirty = true;

        for (const SceneNode* const child : _children)
        {
            child->MarkDirty();
        }
    }

    void SceneNode::UpdateMatrices() const
    {
        TransformComponent& transform = _entityRegistry->get<TransformComponent>(_entityId);

        UpdateMatrices(transform);
    }

    void SceneNode::UpdateMatrices(TransformComponent& Transform) const
    {
        glm::mat4& localToParent = Transform._localToParentMatrix;

        localToParent = glm::translate(glm::mat4(1.0), Transform._position);

        localToParent *= glm::toMat4(Transform._rotation);

        localToParent = glm::scale(localToParent, Transform._scale);

        if (_parent)
        {
            TransformComponent& parentTransform = _parent->_entityRegistry->get<TransformComponent>(_parent->_entityId);
            if (parentTransform._isDirty)
            {
                _parent->UpdateMatrices(parentTransform);
            }
            Transform._localToWorldMatrix = parentTransform._localToWorldMatrix * localToParent;
        } else
        {
            Transform._localToWorldMatrix = localToParent;
        }
        Transform._isDirty = false;
    }
}
