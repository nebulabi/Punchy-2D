#include "PhysicsEngine.h"
#include <random>

namespace Punchy2D {

static std::mt19937 rng(42);
static std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

BodyId PhysicsEngine::createObject() {
    Body b;
    b.id = nextBodyId_++;
    b.setMass(1.f);
    bodies_.push_back(b);
    return b.id;
}

BodyId PhysicsEngine::createStaticBody(Vec2 position, float radius) {
    Body b;
    b.id = nextBodyId_++;
    b.position = position;
    b.radius = radius;
    b.isStatic = true;
    b.mass = 0.f;
    b.invMass = 0.f;
    bodies_.push_back(b);
    return b.id;
}

void PhysicsEngine::removeObject(BodyId id) {
    bodies_.erase(
        std::remove_if(bodies_.begin(), bodies_.end(),
                       [id](const Body& b) { return b.id == id; }),
        bodies_.end());
    constraints_.erase(
        std::remove_if(constraints_.begin(), constraints_.end(),
                       [id](const Constraint& c) {
                           return c.bodyA == id || c.bodyB == id;
                       }),
        constraints_.end());
}

Body* PhysicsEngine::getBody(BodyId id) {
    for (auto& b : bodies_) if (b.id == id) return &b;
    return nullptr;
}

const Body* PhysicsEngine::getBody(BodyId id) const {
    for (auto& b : bodies_) if (b.id == id) return &b;
    return nullptr;
}

ConstraintId PhysicsEngine::addConstraint(const Constraint& c) {
    Constraint copy = c;
    copy.id = nextConstraintId_++;
    constraints_.push_back(copy);
    return copy.id;
}

void PhysicsEngine::removeConstraint(ConstraintId id) {
    constraints_.erase(
        std::remove_if(constraints_.begin(), constraints_.end(),
                       [id](const Constraint& c) { return c.id == id; }),
        constraints_.end());
}

void PhysicsEngine::applyForce(BodyId id, Vec2 force) {
    if (Body* b = getBody(id); b && !b->isStatic) {
        b->velocity += force * b->invMass;
    }
}

void PhysicsEngine::applyImpulse(BodyId id, Vec2 impulse, Vec2 worldPoint) {
    Body* b = getBody(id);
    if (!b || b->isStatic) return;

    impulse *= cfg_.impulseMultiplier;

    Vec2 chaos = Vec2{dist(rng), dist(rng)} * cfg_.chaosMultiplier * impulse.length();
    impulse += chaos;

    b->velocity += impulse * b->invMass;

    Vec2 r = worldPoint - b->position;
    float torque = r.cross(impulse) * b->invInertia * cfg_.spinMultiplier;
    b->angularVel += torque;
}

void PhysicsEngine::applyAngularImpulse(BodyId id, float impulse) {
    if (Body* b = getBody(id); b && !b->isStatic) {
        b->angularVel += impulse * b->invInertia * cfg_.spinMultiplier;
    }
}

void PhysicsEngine::update(float dt) {
    if (dt > cfg_.maxDeltaTime) {
        dt = cfg_.maxDeltaTime;
    }

    accumulator_ += dt;
    float fixedDt = 1.0f / cfg_.ticksPerSecond;

    int maxSteps = 10;
    int steps = 0;

    while (accumulator_ >= fixedDt && steps < maxSteps) {
        fixedUpdate(fixedDt);
        accumulator_ -= fixedDt;
        totalTicks_++;
        steps++;
    }

    if (accumulator_ < 0.f) {
        accumulator_ = 0.f;
    }
}

void PhysicsEngine::reset() {
    accumulator_ = 0.f;
    totalTicks_ = 0;
}

void PhysicsEngine::setTicksPerSecond(float tps) {
    float oldFixedDt = 1.0f / cfg_.ticksPerSecond;
    cfg_.ticksPerSecond = tps;
    float newFixedDt = 1.0f / cfg_.ticksPerSecond;

    if (accumulator_ > 0.f && oldFixedDt > 0.f) {
        float ticksInAccumulator = accumulator_ / oldFixedDt;
        accumulator_ = ticksInAccumulator * newFixedDt;
    }
}

int PhysicsEngine::getAccumulatorTicks() const {
    float fixedDt = 1.0f / cfg_.ticksPerSecond;
    if (fixedDt <= 0.f) return 0;
    return static_cast<int>(accumulator_ / fixedDt);
}

void PhysicsEngine::fixedUpdate(float fixedDt) {
    float subDt = fixedDt / cfg_.substeps;
    for (int s = 0; s < cfg_.substeps; ++s) {
        integrateVelocities(subDt);
        solveConstraints();
        solveCollisions();
        integratePositions(subDt);
        addChaos();
        clampVelocities();
    }
}

void PhysicsEngine::integrateVelocities(float dt) {
    for (auto& b : bodies_) {
        if (b.isStatic || !b.active) continue;
        b.velocity   += cfg_.gravity * dt;
        b.velocity   *= b.drag;
        b.angularVel *= b.angularDrag;
    }
}

void PhysicsEngine::integratePositions(float dt) {
    for (auto& b : bodies_) {
        if (b.isStatic || !b.active) continue;
        b.position += b.velocity * dt;
        b.rotation += b.angularVel * dt;
    }
}

void PhysicsEngine::solveConstraints() {
    for (int iter = 0; iter < cfg_.constraintIterations; ++iter) {
        for (auto& c : constraints_) {
            if (!c.active || c.broken) continue;

            Body* a = getBody(c.bodyA);
            Body* b = getBody(c.bodyB);
            if (!a || !b) continue;

            Vec2 wA = worldAnchor(*a, c.anchorA);
            Vec2 wB = worldAnchor(*b, c.anchorB);
            Vec2 delta = wB - wA;
            float dist = delta.length();
            if (dist < 1e-6f) continue;

            float diff = (dist - c.restLength) / dist;
            Vec2 correction = delta * diff * c.stiffness;

            float forceEst = diff * dist * (a->mass + b->mass);
            if (forceEst > c.maxForce) { c.broken = true; continue; }

            float totalInv = a->invMass + b->invMass;
            if (totalInv < 1e-8f) continue;

            float ratioA = a->invMass / totalInv;
            float ratioB = b->invMass / totalInv;

            if (!a->isStatic) a->position += correction * ratioA;
            if (!b->isStatic) b->position -= correction * ratioB;

            if (iter == 0) {
                Vec2 relVel = b->velocity - a->velocity;
                Vec2 chainImpulse = relVel * cfg_.chainReactionScale * c.stiffness;
                if (!a->isStatic) a->velocity += chainImpulse * ratioA;
                if (!b->isStatic) b->velocity -= chainImpulse * ratioB;
            }
        }
    }
}

void PhysicsEngine::solveCollisions() {
    for (size_t i = 0; i < bodies_.size(); ++i) {
        for (size_t j = i + 1; j < bodies_.size(); ++j) {
            Body& a = bodies_[i];
            Body& b = bodies_[j];
            if (!a.active || !b.active) continue;
            if (a.isStatic && b.isStatic) continue;

            Vec2 delta = b.position - a.position;
            float dist = delta.length();
            float minDist = a.radius + b.radius;

            if (dist < minDist && dist > 1e-6f) {
                Vec2 normal = delta / dist;
                float penetration = minDist - dist;

                float totalInv = a.invMass + b.invMass;
                if (totalInv > 1e-8f) {
                    Vec2 sep = normal * (penetration / totalInv);
                    if (!a.isStatic) a.position -= sep * a.invMass;
                    if (!b.isStatic) b.position += sep * b.invMass;
                }

                Vec2 relVel = a.velocity - b.velocity;
                float velAlongNormal = relVel.dot(normal);
                if (velAlongNormal > 0.f) continue;

                float e = std::max(a.restitution, b.restitution);
                float j = -(1.f + e) * velAlongNormal / totalInv;
                Vec2 impulse = normal * j;

                if (!a.isStatic) a.velocity += impulse * a.invMass;
                if (!b.isStatic) b.velocity -= impulse * b.invMass;

                Vec2 chaos = Vec2{dist(rng), dist(rng)} * cfg_.chaosMultiplier * impulse.length() * 0.5f;
                if (!a.isStatic) a.velocity += chaos;
                if (!b.isStatic) b.velocity -= chaos;

                Vec2 rA = normal * a.radius;
                Vec2 rB = normal * -b.radius;
                if (!a.isStatic)
                    a.angularVel += rA.cross(impulse) * a.invInertia * cfg_.spinMultiplier * 0.5f;
                if (!b.isStatic)
                    b.angularVel -= rB.cross(impulse) * b.invInertia * cfg_.spinMultiplier * 0.5f;
            }
        }
    }
}

void PhysicsEngine::addChaos() {
    for (auto& b : bodies_) {
        if (b.isStatic || !b.active) continue;
        float speed = b.velocity.length();
        if (speed > 500.f && speed < 3000.f) {
            Vec2 chaos = Vec2{dist(rng), dist(rng)} * cfg_.chaosMultiplier * 1.5f;
            b.velocity += chaos;
            b.angularVel += dist(rng) * cfg_.chaosMultiplier * 2.f;
        }
    }
}

void PhysicsEngine::clampVelocities() {
    float maxV = cfg_.maxVelocity;
    float maxV2 = maxV * maxV;
    float maxW = cfg_.maxAngularVel;

    for (auto& b : bodies_) {
        if (b.velocity.lengthSq() > maxV2)
            b.velocity = b.velocity.normalized() * maxV;
        if (std::abs(b.angularVel) > maxW)
            b.angularVel = (b.angularVel > 0 ? 1 : -1) * maxW;
    }
}

}