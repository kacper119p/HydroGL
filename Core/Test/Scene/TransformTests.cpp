#include <entt/entity/registry.hpp>
#include <glm/gtx/quaternion.hpp>
#include <gtest/gtest.h>

#include "../TestsCommonUtilities/TestChecks.h"
#include "Scene/TransformComponent.h"


class TransformTests : public testing::Test
{
protected:
    TransformTests() = default;

    std::unique_ptr<entt::registry> _registry = std::make_unique<entt::registry>();
};


// Demonstrate some basic assertions.
TEST_F(TransformTests, TransformCreationAndDestruction)
{
    const entt::entity id = _registry->create();
    ASSERT_NO_FATAL_FAILURE(_registry->emplace<Core::TransformComponent>(id));
    ASSERT_NO_FATAL_FAILURE(_registry->remove<Core::TransformComponent>(id));
}

TEST_F(TransformTests, TransformWitchChildCreationAndDestruction)
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


TEST_F(TransformTests, TransformMatrixUpdate)
{
    const ::entt::entity id = _registry->create();
    Core::TransformComponent& transform = _registry->emplace<::Core::TransformComponent>(id);

    // Euler Angles
    constexpr glm::vec3 euler = glm::vec3(90.0f, 45.0f, 12.0f);
    glm::mat4 targetMatrix = glm::toMat4(glm::quat(euler));
    ASSERT_NO_FATAL_FAILURE(transform.SetEulerAngles(euler));
    ASSERT_NO_FATAL_FAILURE(::TestUtilities::AssertMat4Equal(transform.GetLocalToParentMatrix(), targetMatrix));
    ASSERT_NO_FATAL_FAILURE(::TestUtilities::AssertMat4Equal(transform.GetLocalToWorldMatrix(), targetMatrix));

    ASSERT_NO_FATAL_FAILURE(transform.SetEulerAngles(glm::vec3(0.0f)));

    // Translation
    glm::vec3 position = glm::vec3(326.0f, -7235.0f, 523.0f);
    targetMatrix = glm::translate(glm::mat4(1.0f), position);
    ASSERT_NO_FATAL_FAILURE(transform.SetPosition(position));

    ASSERT_NO_FATAL_FAILURE(::TestUtilities::AssertMat4Equal(transform.GetLocalToParentMatrix(), targetMatrix));
    ASSERT_NO_FATAL_FAILURE(::TestUtilities::AssertMat4Equal(transform.GetLocalToWorldMatrix(), targetMatrix));

    // Quat rotation
    const glm::quat rotation = glm::quat(glm::vec3(53.0f, -53.0f, 43.0f));
    targetMatrix *= glm::toMat4(rotation);
    transform.SetRotation(rotation);

    ASSERT_NO_FATAL_FAILURE(::TestUtilities::AssertMat4Equal(transform.GetLocalToParentMatrix(), targetMatrix));
    ASSERT_NO_FATAL_FAILURE(::TestUtilities::AssertMat4Equal(transform.GetLocalToWorldMatrix(), targetMatrix));

    // Scale
    constexpr glm::vec3 scale = glm::vec3(6.0f, 3.0f, 64.0f);
    targetMatrix = glm::scale(targetMatrix, scale);
    ASSERT_NO_FATAL_FAILURE(transform.SetScale(scale));

    ASSERT_NO_FATAL_FAILURE(::TestUtilities::AssertMat4Equal(transform.GetLocalToParentMatrix(), targetMatrix));
    ASSERT_NO_FATAL_FAILURE(::TestUtilities::AssertMat4Equal(transform.GetLocalToWorldMatrix(), targetMatrix));
}

TEST_F(TransformTests, ChildMatrixUpdateTest)
{
    const ::entt::entity parentId = _registry->create();
    Core::TransformComponent& parentTransform = _registry->emplace<::Core::TransformComponent>(parentId);
    const ::entt::entity childId = _registry->create();
    Core::TransformComponent& childTransform = _registry->emplace<::Core::TransformComponent>(childId);

    ASSERT_NO_FATAL_FAILURE(childTransform.GetSceneHierarchyNode().SetParent(parentTransform.GetSceneHierarchyNode()));
    ASSERT_NO_FATAL_FAILURE(
        ::TestUtilities::AssertMat4Equal(childTransform.GetLocalToWorldMatrix(), glm::identity<glm::mat4>()));
    ASSERT_NO_FATAL_FAILURE(
        ::TestUtilities::AssertMat4Equal(childTransform.GetLocalToParentMatrix(), glm::identity<glm::mat4>()));

    constexpr glm::vec3 position = glm::vec3(3420.0f, 53.0f, 53.0f);
    constexpr glm::mat4 targetMatrix = glm::translate(glm::mat4(1.0f), position);
    ASSERT_NO_FATAL_FAILURE(parentTransform.SetPosition(position));

    ASSERT_NO_FATAL_FAILURE(::TestUtilities::AssertMat4Equal(childTransform.GetLocalToWorldMatrix(), targetMatrix));
    ASSERT_NO_FATAL_FAILURE(
        ::TestUtilities::AssertMat4Equal(childTransform.GetLocalToParentMatrix(), glm::identity<glm::mat4>()));
}
