#pragma once
#include <Jolt/Jolt.h>
#include <Jolt/Core/Reference.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>

#include "Meshes/Mesh.h"

namespace Core::MeshColliderFactory
{
    JPH::RefConst<JPH::Shape> CreateConvexHullCollider(const Mesh& Mesh, float Density);
}
