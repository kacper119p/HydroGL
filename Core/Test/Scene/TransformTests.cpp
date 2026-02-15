#include <entt/entity/registry.hpp>
#include <glm/gtx/quaternion.hpp>
#include <gtest/gtest.h>

#include "Scene/TransformComponent.h"
#include "TestsCommonUtilities/TestChecks.h"


class TransformTests : public testing::Test
{
protected:
    TransformTests() = default;

    std::unique_ptr<entt::registry> _registry = std::make_unique<entt::registry>();
};

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

    ASSERT_NO_FATAL_FAILURE(parentComponent.GetSceneHierarchyNode()->AddChild(*childComponent.GetSceneHierarchyNode()));

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
    glm::mat4 targetMatrix = glm::toMat4(glm::quat(glm::radians(euler)));
    ASSERT_NO_FATAL_FAILURE(transform.SetLocalEulerAngles(euler));
    ASSERT_NO_FATAL_FAILURE(::TestUtilities::AssertMat4Equal(transform.GetLocalToParentMatrix(), targetMatrix));
    ASSERT_NO_FATAL_FAILURE(::TestUtilities::AssertMat4Equal(transform.GetLocalToWorldMatrix(), targetMatrix));

    ASSERT_NO_FATAL_FAILURE(transform.SetLocalEulerAngles(glm::vec3(0.0f)));

    // Translation
    glm::vec3 position = glm::vec3(326.0f, -7235.0f, 523.0f);
    targetMatrix = glm::translate(glm::mat4(1.0f), position);
    ASSERT_NO_FATAL_FAILURE(transform.SetLocalPosition(position));

    ASSERT_NO_FATAL_FAILURE(::TestUtilities::AssertMat4Equal(transform.GetLocalToParentMatrix(), targetMatrix));
    ASSERT_NO_FATAL_FAILURE(::TestUtilities::AssertMat4Equal(transform.GetLocalToWorldMatrix(), targetMatrix));

    // Quat rotation
    const glm::quat rotation = glm::quat(glm::vec3(53.0f, -53.0f, 43.0f));
    targetMatrix *= glm::toMat4(rotation);
    transform.SetLocalRotation(rotation);

    ASSERT_NO_FATAL_FAILURE(::TestUtilities::AssertMat4Equal(transform.GetLocalToParentMatrix(), targetMatrix));
    ASSERT_NO_FATAL_FAILURE(::TestUtilities::AssertMat4Equal(transform.GetLocalToWorldMatrix(), targetMatrix));

    // Scale
    constexpr glm::vec3 scale = glm::vec3(6.0f, 3.0f, 64.0f);
    targetMatrix = glm::scale(targetMatrix, scale);
    ASSERT_NO_FATAL_FAILURE(transform.SetLocalScale(scale));

    ASSERT_NO_FATAL_FAILURE(::TestUtilities::AssertMat4Equal(transform.GetLocalToParentMatrix(), targetMatrix));
    ASSERT_NO_FATAL_FAILURE(::TestUtilities::AssertMat4Equal(transform.GetLocalToWorldMatrix(), targetMatrix));
}

TEST_F(TransformTests, ChildMatrixUpdateTest)
{
    const ::entt::entity parentId = _registry->create();
    Core::TransformComponent& parentTransform = _registry->emplace<::Core::TransformComponent>(parentId);
    const ::entt::entity childId = _registry->create();
    Core::TransformComponent& childTransform = _registry->emplace<::Core::TransformComponent>(childId);

    ASSERT_NO_FATAL_FAILURE(childTransform.GetSceneHierarchyNode()->SetParent(parentTransform.GetSceneHierarchyNode()));
    ASSERT_NO_FATAL_FAILURE(
        ::TestUtilities::AssertMat4Equal(childTransform.GetLocalToWorldMatrix(), glm::identity<glm::mat4>()));
    ASSERT_NO_FATAL_FAILURE(
        ::TestUtilities::AssertMat4Equal(childTransform.GetLocalToParentMatrix(), glm::identity<glm::mat4>()));

    constexpr glm::vec3 position = glm::vec3(3420.0f, 53.0f, 53.0f);
    constexpr glm::mat4 targetMatrix = glm::translate(glm::mat4(1.0f), position);
    ASSERT_NO_FATAL_FAILURE(parentTransform.SetLocalPosition(position));

    ASSERT_NO_FATAL_FAILURE(::TestUtilities::AssertMat4Equal(childTransform.GetLocalToWorldMatrix(), targetMatrix));
    ASSERT_NO_FATAL_FAILURE(
        ::TestUtilities::AssertMat4Equal(childTransform.GetLocalToParentMatrix(), glm::identity<glm::mat4>()));
}

TEST_F(TransformTests, GetWorldPosition)
{
    const entt::entity id = _registry->create();
    Core::TransformComponent& transform = _registry->emplace<Core::TransformComponent>(id);

    constexpr glm::vec3 position(10.0f, 20.0f, 30.0f);
    transform.SetLocalPosition(position);

    ASSERT_EQ(transform.GetWorldPosition(), position);
}

TEST_F(TransformTests, SetWorldPosition)
{
    const entt::entity id = _registry->create();
    Core::TransformComponent& transform = _registry->emplace<Core::TransformComponent>(id);

    constexpr glm::vec3 worldPos(100.0f, 200.0f, 300.0f);
    transform.SetWorldPosition(worldPos);

    ASSERT_EQ(transform.GetWorldPosition(), worldPos);
    ASSERT_EQ(transform.GetLocalPosition(), worldPos);
}

TEST_F(TransformTests, SetWorldPositionWithParent)
{
    const entt::entity parentId = _registry->create();
    Core::TransformComponent& parentTransform = _registry->emplace<Core::TransformComponent>(parentId);
    const entt::entity childId = _registry->create();
    Core::TransformComponent& childTransform = _registry->emplace<Core::TransformComponent>(childId);

    childTransform.GetSceneHierarchyNode()->SetParent(parentTransform.GetSceneHierarchyNode());

    constexpr glm::vec3 parentPos(100.0f, 0.0f, 0.0f);
    parentTransform.SetLocalPosition(parentPos);

    constexpr glm::vec3 targetWorldPos(150.0f, 50.0f, 25.0f);
    childTransform.SetWorldPosition(targetWorldPos);

    ASSERT_NO_FATAL_FAILURE(::TestUtilities::AssertVec3Equal(childTransform.GetWorldPosition(), targetWorldPos));
}

TEST_F(TransformTests, GetWorldRotation)
{
    const entt::entity id = _registry->create();
    Core::TransformComponent& transform = _registry->emplace<Core::TransformComponent>(id);

    const glm::quat rotation = glm::quat(glm::radians(glm::vec3(45.0f, 30.0f, 60.0f)));
    transform.SetLocalRotation(rotation);

    const glm::quat worldRot = transform.GetWorldRotation();
    ASSERT_NO_FATAL_FAILURE(::TestUtilities::AssertQuatEqual(worldRot, rotation));
}

TEST_F(TransformTests, SetWorldRotation)
{
    const entt::entity id = _registry->create();
    Core::TransformComponent& transform = _registry->emplace<Core::TransformComponent>(id);

    const glm::quat rotation = glm::quat(glm::radians(glm::vec3(90.0f, 45.0f, 0.0f)));
    transform.SetWorldRotation(rotation);

    ASSERT_NO_FATAL_FAILURE(::TestUtilities::AssertQuatEqual(transform.GetWorldRotation(), rotation));
}

TEST_F(TransformTests, SetWorldRotationWithParent)
{
    const entt::entity parentId = _registry->create();
    Core::TransformComponent& parentTransform = _registry->emplace<Core::TransformComponent>(parentId);
    const entt::entity childId = _registry->create();
    Core::TransformComponent& childTransform = _registry->emplace<Core::TransformComponent>(childId);

    childTransform.GetSceneHierarchyNode()->SetParent(parentTransform.GetSceneHierarchyNode());

    const glm::quat parentRot = glm::quat(glm::radians(glm::vec3(45.0f, 0.0f, 0.0f)));
    parentTransform.SetLocalRotation(parentRot);

    const glm::quat targetWorldRot = glm::quat(glm::radians(glm::vec3(90.0f, 30.0f, 0.0f)));
    childTransform.SetWorldRotation(targetWorldRot);

    ASSERT_NO_FATAL_FAILURE(::TestUtilities::AssertQuatEqual(childTransform.GetWorldRotation(), targetWorldRot));
}

TEST_F(TransformTests, GetWorldEulerAngles)
{
    const entt::entity id = _registry->create();
    Core::TransformComponent& transform = _registry->emplace<Core::TransformComponent>(id);

    constexpr glm::vec3 euler(45.0f, 30.0f, 60.0f);
    transform.SetLocalEulerAngles(euler);

    const glm::vec3 worldEuler = transform.GetWorldEulerAngles();
    ASSERT_NO_FATAL_FAILURE(::TestUtilities::AssertVec3Equal(worldEuler, euler));
}

TEST_F(TransformTests, SetWorldEulerAngles)
{
    const entt::entity id = _registry->create();
    Core::TransformComponent& transform = _registry->emplace<Core::TransformComponent>(id);

    constexpr glm::vec3 euler(90.0f, 45.0f, 30.0f);
    transform.SetWorldEulerAngles(euler);

    const glm::vec3 worldEuler = transform.GetWorldEulerAngles();
    ASSERT_NO_FATAL_FAILURE(::TestUtilities::AssertVec3Equal(worldEuler, euler));
}

TEST_F(TransformTests, GetWorldDirectionVectors)
{
    const entt::entity id = _registry->create();
    Core::TransformComponent& transform = _registry->emplace<Core::TransformComponent>(id);

    const glm::vec3 right = transform.GetWorldRight();
    const glm::vec3 up = transform.GetWorldUp();
    const glm::vec3 forward = transform.GetWorldForward();

    ASSERT_NO_FATAL_FAILURE(::TestUtilities::AssertVec3Equal(right, glm::vec3(1.0f, 0.0f, 0.0f)));
    ASSERT_NO_FATAL_FAILURE(::TestUtilities::AssertVec3Equal(up, glm::vec3(0.0f, 1.0f, 0.0f)));
    ASSERT_NO_FATAL_FAILURE(::TestUtilities::AssertVec3Equal(forward, glm::vec3(0.0f, 0.0f, -1.0f)));
}

TEST_F(TransformTests, GetWorldDirectionVectorsWithRotation)
{
    const entt::entity id = _registry->create();
    Core::TransformComponent& transform = _registry->emplace<Core::TransformComponent>(id);

    transform.SetLocalEulerAngles(glm::vec3(0.0f, 90.0f, 0.0f));

    const glm::vec3 forward = transform.GetWorldForward();
    ASSERT_NO_FATAL_FAILURE(::TestUtilities::AssertVec3Equal(forward, glm::vec3(-1.0f, 0.0f, 0.0f)));
}

TEST_F(TransformTests, GetWorldToLocalMatrix)
{
    const entt::entity id = _registry->create();
    Core::TransformComponent& transform = _registry->emplace<Core::TransformComponent>(id);

    transform.SetLocalPosition(glm::vec3(10.0f, 20.0f, 30.0f));

    const glm::mat4 worldToLocal = transform.GetWorldToLocalMatrix();
    const glm::mat4 localToWorld = transform.GetLocalToWorldMatrix();
    const glm::mat4 identity = worldToLocal * localToWorld;

    ASSERT_NO_FATAL_FAILURE(::TestUtilities::AssertMat4Equal(identity, glm::identity<glm::mat4>()));
}

TEST_F(TransformTests, GetParentToLocalMatrix)
{
    const entt::entity id = _registry->create();
    Core::TransformComponent& transform = _registry->emplace<Core::TransformComponent>(id);

    transform.SetLocalPosition(glm::vec3(5.0f, 10.0f, 15.0f));

    const glm::mat4 parentToLocal = transform.GetParentToLocalMatrix();
    const glm::mat4 localToParent = transform.GetLocalToParentMatrix();
    const glm::mat4 identity = parentToLocal * localToParent;

    ASSERT_NO_FATAL_FAILURE(::TestUtilities::AssertMat4Equal(identity, glm::identity<glm::mat4>()));
}

TEST_F(TransformTests, MultiLevelHierarchyMatrixUpdate)
{
    const entt::entity grandparentId = _registry->create();
    Core::TransformComponent& grandparent = _registry->emplace<Core::TransformComponent>(grandparentId);

    const entt::entity parentId = _registry->create();
    Core::TransformComponent& parent = _registry->emplace<Core::TransformComponent>(parentId);

    const entt::entity childId = _registry->create();
    Core::TransformComponent& child = _registry->emplace<Core::TransformComponent>(childId);

    parent.GetSceneHierarchyNode()->SetParent(grandparent.GetSceneHierarchyNode());
    child.GetSceneHierarchyNode()->SetParent(parent.GetSceneHierarchyNode());

    grandparent.SetLocalPosition(glm::vec3(100.0f, 0.0f, 0.0f));
    parent.SetLocalPosition(glm::vec3(50.0f, 0.0f, 0.0f));
    child.SetLocalPosition(glm::vec3(25.0f, 0.0f, 0.0f));

    const glm::vec3 expectedWorldPos(175.0f, 0.0f, 0.0f);
    ASSERT_NO_FATAL_FAILURE(::TestUtilities::AssertVec3Equal(child.GetWorldPosition(), expectedWorldPos));
}

TEST_F(TransformTests, RemoveChildUpdatesHierarchy)
{
    const entt::entity parentId = _registry->create();
    Core::TransformComponent& parentTransform = _registry->emplace<Core::TransformComponent>(parentId);

    const entt::entity childId = _registry->create();
    Core::TransformComponent& childTransform = _registry->emplace<Core::TransformComponent>(childId);

    parentTransform.GetSceneHierarchyNode()->AddChild(*childTransform.GetSceneHierarchyNode());
    ASSERT_EQ(childTransform.GetSceneHierarchyNode()->GetParent(), parentTransform.GetSceneHierarchyNode());

    parentTransform.GetSceneHierarchyNode()->RemoveChild(*childTransform.GetSceneHierarchyNode());
    ASSERT_EQ(childTransform.GetSceneHierarchyNode()->GetParent(), nullptr);
}

