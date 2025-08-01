#include <entt/entity/registry.hpp>
#include <gtest/gtest.h>

#include "Scene/TransformComponent.h"


class TestDriver : public testing::Test
{
protected:
    TestDriver() {}

    std::unique_ptr<entt::registry> _registry = std::make_unique<entt::registry>();
};


// Demonstrate some basic assertions.
TEST_F(TestDriver, TransformCreationAndDestruction)
{
    const entt::entity id = _registry->create();
    ASSERT_NO_FATAL_FAILURE(_registry->emplace<Core::TransformComponent>(id));
    ASSERT_NO_FATAL_FAILURE(_registry->remove<Core::TransformComponent>(id));
}

TEST_F(TestDriver, TransformWitchChildCreationAndDestruction)
{
    const ::entt::entity parentId = _registry->create();
    const ::entt::entity childId = _registry->create();
    ASSERT_NO_FATAL_FAILURE(_registry->emplace<Core::TransformComponent>(parentId));
    ASSERT_NO_FATAL_FAILURE(_registry->emplace<Core::TransformComponent>(childId));

    const ::Core::TransformComponent& parentComponent = _registry->get<Core::TransformComponent>(parentId);
    const ::Core::TransformComponent& childComponent = _registry->get<Core::TransformComponent>(childId);

    ASSERT_NO_FATAL_FAILURE(parentComponent.GetSceneHierarchyNode().AddChild(childComponent.GetSceneHierarchyNode()));

    ASSERT_NO_FATAL_FAILURE(_registry->destroy(parentId));
    ASSERT_FALSE(_registry->valid(parentId));
    ASSERT_FALSE(_registry->valid(childId));
}


TEST_F(TestDriver, TransformMatrixUpdate)
{
    const ::entt::entity id = _registry->create();
    ASSERT_NO_FATAL_FAILURE(_registry->emplace<Core::TransformComponent>(id));
    ASSERT_NO_FATAL_FAILURE(_registry->remove<Core::TransformComponent>(id));
}
