#pragma once
#include <vector>
#include <cstdint>
#include <cmath>
#include <algorithm>

namespace Punchy2D {

struct Vec2 {
    float x = 0, y = 0;

    Vec2 operator+(Vec2 o) const { return {x + o.x, y + o.y}; }
    Vec2 operator-(Vec2 o) const { return {x - o.x, y - o.y}; }
    Vec2 operator*(float s) const { return {x * s, y * s}; }
    Vec2 operator/(float s) const { return {x / s, y / s}; }
    Vec2& operator+=(Vec2 o) { x += o.x; y += o.y; return *this; }
    Vec2& operator-=(Vec2 o) { x -= o.x; y -= o.y; return *this; }
    Vec2& operator*=(float s) { x *= s; y *= s; return *this; }

    float dot(Vec2 o)   const { return x * o.x + y * o.y; }
    float cross(Vec2 o)  const { return x * o.y - y * o.x; }
    float lengthSq()     const { return x * x + y * y; }
    float length()       const { return std::sqrt(lengthSq()); }
    Vec2  normalized()   const {
        float l = length();
        return l > 1e-6f ? *this / l : Vec2{0, 0};
    }
    Vec2 perp() const { return {-y, x}; }
};
inline Vec2 operator*(float s, Vec2 v) { return v * s; }

using BodyId       = uint32_t;
using ConstraintId = uint32_t;
constexpr BodyId INVALID_BODY = 0;

struct Body {
    BodyId  id             = INVALID_BODY;
    Vec2    position;
    Vec2    velocity;
    float   rotation       = 0.f;
    float   angularVel     = 0.f;
    float   mass           = 1.f;
    float   invMass        = 1.f;
    float   radius         = 10.f;
    float   restitution    = 0.9f;
    float   drag           = 0.998f;
    float   angularDrag    = 0.98f;
    float   inertia        = 1.f;
    float   invInertia     = 1.f;
    bool    isStatic       = false;
    bool    active         = true;

    void setMass(float m) {
        mass     = m;
        invMass  = (m > 0.f) ? 1.f / m : 0.f;
        inertia  = 0.5f * m * radius * radius;
        invInertia = (inertia > 0.f) ? 1.f / inertia : 0.f;
    }
};

struct Constraint {
    ConstraintId id        = 0;
    BodyId       bodyA     = INVALID_BODY;
    BodyId       bodyB     = INVALID_BODY;
    Vec2         anchorA;
    Vec2         anchorB;
    float        restLength = 0.f;
    float        stiffness  = 0.3f;
    float        maxForce   = 1e8f;
    bool         active    = true;
    bool         broken    = false;
};

struct PhysicsConfig {
    Vec2  gravity              = {0.f, 300.f};
    float ticksPerSecond       = 35.f;
    float maxDeltaTime         = 0.25f;
    int   constraintIterations = 5;
    int   substeps             = 2;
    float impulseMultiplier    = 3.0f;
    float spinMultiplier       = 5.0f;
    float chainReactionScale   = 0.8f;
    float chaosMultiplier      = 0.3f;
    float maxVelocity          = 8000.f;
    float maxAngularVel        = 100.f;
};

class PhysicsEngine {
public:
    PhysicsEngine() = default;

    BodyId       createObject();
    BodyId       createStaticBody(Vec2 position, float radius);
    void         removeObject(BodyId id);
    Body*        getBody(BodyId id);
    const Body*  getBody(BodyId id) const;

    ConstraintId addConstraint(const Constraint& c);
    void         removeConstraint(ConstraintId id);

    void applyForce(BodyId id, Vec2 force);
    void applyImpulse(BodyId id, Vec2 impulse, Vec2 worldPoint);
    void applyAngularImpulse(BodyId id, float impulse);

    void update(float dt);
    void reset();

    float getTicksPerSecond() const { return cfg_.ticksPerSecond; }
    void setTicksPerSecond(float tps);
    int getAccumulatorTicks() const;
    int getTotalTicks() const { return totalTicks_; }

    PhysicsConfig&       config()       { return cfg_; }
    const PhysicsConfig& config() const { return cfg_; }
    std::vector<Body>&       bodies()       { return bodies_; }
    const std::vector<Body>& bodies() const { return bodies_; }
    std::vector<Constraint>&       constraints()       { return constraints_; }
    const std::vector<Constraint>& constraints() const { return constraints_; }

private:
    void fixedUpdate(float fixedDt);
    void integrateVelocities(float dt);
    void integratePositions(float dt);
    void solveConstraints();
    void solveCollisions();
    void addChaos();
    void clampVelocities();

    BodyId       nextBodyId_       = 1;
    ConstraintId nextConstraintId_ = 1;
    std::vector<Body>       bodies_;
    std::vector<Constraint> constraints_;
    PhysicsConfig           cfg_;
    float                   accumulator_ = 0.f;
    int                     totalTicks_ = 0;
};

inline Vec2 worldAnchor(const Body& b, Vec2 local) {
    float c = std::cos(b.rotation), s = std::sin(b.rotation);
    return b.position + Vec2{local.x * c - local.y * s,
                             local.x * s + local.y * c};
}

}