#include "TransformComponent.h"

namespace Core
{
    void TransformComponent::on_construct(entt::registry& Registry, const entt::entity Id)
    {
        TransformComponent& transform = Registry.get<TransformComponent>(Id);
        transform._sceneHierarchyNode = new SceneNode(&Registry, Id);
    }
    void TransformComponent::on_destroy(entt::registry& Registry, const entt::entity Id)
    {
        const TransformComponent& transform = Registry.get<TransformComponent>(Id);
        delete transform._sceneHierarchyNode;
    }
} // namespace Core
