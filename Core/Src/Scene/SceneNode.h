#pragma once
#include <entt/entity/registry.hpp>
#include <vector>

#include "boost/pool/object_pool.hpp"
#include "boost/pool/pool.hpp"

namespace Core
{
    struct TransformComponent;

    class SceneNode
    {
    private:
        static boost::object_pool<SceneNode, boost::default_user_allocator_new_delete> _memoryPool;

    private:
        SceneNode* _parent = nullptr;
        std::vector<SceneNode*> _children;
        entt::registry* _entityRegistry;
        entt::entity _entityId;

    public:
        SceneNode(entt::registry* EntityRegistry, const entt::entity EntityId) : _entityRegistry(EntityRegistry),
            _entityId(EntityId)
        {
        }

    public:
        ~SceneNode();

    public:
        static SceneNode* Create(entt::registry* EntityRegistry, entt::entity EntityId);

    public:
        void AddChild(SceneNode& Child)
        {
            if (Child._parent != nullptr)
            {
                Child._parent->RemoveChild(Child);
            }
            Child._parent = this;
            Child.MarkDirty();
            _children.push_back(&Child);
        }

        void RemoveChild(SceneNode& Child)
        {
            if (std::erase(_children, &Child))
            {
                Child._parent = nullptr;
                Child.MarkDirty();
            }
        }

        void SetParent(SceneNode* InParent)
        {
            if (_parent != nullptr)
            {
                _parent->RemoveChild(*this);
            }
            _parent = InParent;
            if (_parent != nullptr)
            {
                _parent->AddChild(*this);
            }
            MarkDirty();
        }

        [[nodiscard]] SceneNode* GetParent() const
        {
            return _parent;
        }

        void MarkDirty() const;

        void UpdateMatrices() const;

        [[nodiscard]] std::vector<SceneNode*>::iterator begin()
        {
            return _children.begin();
        }

        [[nodiscard]] std::vector<SceneNode*>::iterator end()
        {
            return _children.end();
        }

        [[nodiscard]] std::vector<SceneNode*>::const_iterator begin() const
        {
            return _children.begin();
        }

        [[nodiscard]] std::vector<SceneNode*>::const_iterator end() const
        {
            return _children.end();
        }

        [[nodiscard]] const TransformComponent* GetTransformComponent() const;

        [[nodiscard]] TransformComponent* GetTransformComponent();

    private:
        void UpdateMatrices(TransformComponent& Transform) const;
    };
}
