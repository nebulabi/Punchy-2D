#pragma once
#include "PhysicsEngine.h"
#include <array>

namespace Punchy2D {

enum RagdollPart : int {
    HEAD = 0,
    TORSO,
    UPPER_ARM_L, LOWER_ARM_L,
    UPPER_ARM_R, LOWER_ARM_R,
    UPPER_LEG_L, LOWER_LEG_L,
    UPPER_LEG_R, LOWER_LEG_R,
    PART_COUNT
};

struct Ragdoll {
    std::array<BodyId, PART_COUNT> parts{};
    BodyId operator[](RagdollPart p) const { return parts[p]; }
};

Ragdoll createRagdoll(PhysicsEngine& engine, Vec2 origin, float scale = 1.f);
void hitRagdollPart(PhysicsEngine& engine, const Ragdoll& rag,
                    RagdollPart part, Vec2 impulse, Vec2 point);

}