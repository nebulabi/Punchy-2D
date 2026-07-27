#include "Ragdoll.h"

namespace Punchy2D {

namespace {

struct PartDef {
    Vec2  offset;
    float radius;
    float mass;
    float restitution;
};

struct JointDef {
    RagdollPart a, b;
};

PartDef defaultParts[PART_COUNT] = {
    {{  0, -90}, 20, 2.0f, 0.95f},
    {{  0, -30}, 25, 4.0f, 0.9f},
    {{-35, -55}, 10, 1.0f, 0.9f},
    {{-55, -35},  9, 0.8f, 0.95f},
    {{ 35, -55}, 10, 1.0f, 0.9f},
    {{ 55, -35},  9, 0.8f, 0.95f},
    {{-16,  10}, 11, 1.5f, 0.9f},
    {{-16,  45}, 10, 1.2f, 0.95f},
    {{ 16,  10}, 11, 1.5f, 0.9f},
    {{ 16,  45}, 10, 1.2f, 0.95f},
};

JointDef defaultJoints[] = {
    {HEAD,         TORSO},
    {TORSO,        UPPER_ARM_L},
    {UPPER_ARM_L,  LOWER_ARM_L},
    {TORSO,        UPPER_ARM_R},
    {UPPER_ARM_R,  LOWER_ARM_R},
    {TORSO,        UPPER_LEG_L},
    {UPPER_LEG_L,  LOWER_LEG_L},
    {TORSO,        UPPER_LEG_R},
    {UPPER_LEG_R,  LOWER_LEG_R},
};

}

Ragdoll createRagdoll(PhysicsEngine& engine, Vec2 origin, float scale) {
    Ragdoll rag;

    for (int i = 0; i < PART_COUNT; ++i) {
        BodyId id = engine.createObject();
        Body* b   = engine.getBody(id);
        const auto& def = defaultParts[i];

        b->position    = origin + def.offset * scale;
        b->radius      = def.radius * scale;
        b->restitution = def.restitution;
        b->drag        = 0.999f;
        b->angularDrag = 0.99f;
        b->setMass(def.mass * scale * scale);
        rag.parts[i]   = id;
    }

    for (const auto& jd : defaultJoints) {
        Body* a = engine.getBody(rag[jd.a]);
        Body* b = engine.getBody(rag[jd.b]);

        Constraint c;
        c.bodyA       = rag[jd.a];
        c.bodyB       = rag[jd.b];
        c.anchorA     = (b->position - a->position) * 0.5f;
        c.anchorB     = (a->position - b->position) * 0.5f;
        c.restLength  = (b->position - a->position).length();
        c.stiffness   = 0.3f;
        c.maxForce    = 150000.f * scale;
        engine.addConstraint(c);
    }

    return rag;
}

void hitRagdollPart(PhysicsEngine& engine, const Ragdoll& rag,
                    RagdollPart part, Vec2 impulse, Vec2 point) {
    BodyId id = rag[part];
    engine.applyImpulse(id, impulse, point);
}

}