#include "PhysicsEngine.h"
#include "Ragdoll.h"
#include <cstdio>
#include <chrono>

using namespace Punchy2D;

void printBody(const char* label, const Body& b) {
    std::printf("  %-14s  pos(%7.1f, %7.1f)  vel(%7.1f, %7.1f)  rot=%6.2f  w=%5.2f\n",
                label, b.position.x, b.position.y,
                b.velocity.x, b.velocity.y,
                b.rotation, b.angularVel);
}

int main() {
    PhysicsEngine engine;

    engine.config().impulseMultiplier  = 3.5f;
    engine.config().spinMultiplier     = 6.0f;
    engine.config().chainReactionScale = 0.9f;
    engine.config().chaosMultiplier    = 0.5f;
    engine.config().gravity            = {0.f, 250.f};

    std::printf("=== Punchy2D Engine Demo ===\n\n");

    std::printf("Default TPS: %.0f\n", engine.getTicksPerSecond());
    std::printf("Max delta time: %.3fs\n\n",
                engine.config().maxDeltaTime);

    Ragdoll rag = createRagdoll(engine, {400.f, 300.f}, 1.0f);
    std::printf("Cartoon Ragdoll created!\n\n");

    BodyId ballId = engine.createObject();
    Body* ball    = engine.getBody(ballId);
    ball->position = {-100.f, 200.f};
    ball->radius   = 18.f;
    ball->restitution = 0.95f;
    ball->setMass(15.f);

    std::printf(">>> PUNCH! Ball hits torso! <<<\n\n");
    engine.applyImpulse(ballId, {10000.f, 300.f}, ball->position);

    std::printf("Simulating 1.0 second at 35 TPS...\n");
    float totalTime = 1.0f;
    int frames = 0;

    auto start = std::chrono::high_resolution_clock::now();

    while (totalTime > 0.f) {
        float dt = 1.f / 60.f;
        if (dt > totalTime) dt = totalTime;

        engine.update(dt);
        totalTime -= dt;
        frames++;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::printf("Render frames: %d (at 60 FPS)\n", frames);
    std::printf("Physics ticks: %d (at 35 TPS)\n", engine.getTotalTicks());
    std::printf("Execution time: %lldms\n\n", (long long)duration.count());

    std::printf("Final state:\n");
    printBody("Head",    *engine.getBody(rag[HEAD]));
    printBody("Torso",   *engine.getBody(rag[TORSO]));
    printBody("Arm L",   *engine.getBody(rag[UPPER_ARM_L]));
    printBody("Ball",    *engine.getBody(ballId));
    std::printf("\n");

    std::printf(">>> Changing TPS to 20... <<<\n\n");
    engine.setTicksPerSecond(20.f);
    std::printf("Current TPS: %.0f\n", engine.getTicksPerSecond());
    std::printf("Accumulator ticks: %d\n\n", engine.getAccumulatorTicks());

    std::printf(">>> MEGA UPPERCUT to head! <<<\n\n");
    Vec2 headPos = engine.getBody(rag[HEAD])->position;
    hitRagdollPart(engine, rag, HEAD, {-3000.f, -8000.f}, headPos);

    std::printf("Simulating 0.5 seconds at 20 TPS...\n");
    totalTime = 0.5f;

    start = std::chrono::high_resolution_clock::now();

    while (totalTime > 0.f) {
        float dt = 1.f / 60.f;
        if (dt > totalTime) dt = totalTime;

        engine.update(dt);
        totalTime -= dt;
    }

    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::printf("Execution time: %lldms\n", (long long)duration.count());
    std::printf("Total physics ticks: %d\n\n", engine.getTotalTicks());

    std::printf("After uppercut:\n");
    printBody("Head",    *engine.getBody(rag[HEAD]));
    printBody("Torso",   *engine.getBody(rag[TORSO]));
    printBody("Arm L",   *engine.getBody(rag[UPPER_ARM_L]));
    std::printf("\n");

    std::printf(">>> Testing spiral of death protection <<<\n\n");
    std::printf("Calling update with dt = 2.0s (larger than maxDeltaTime)...\n");
    engine.update(2.0f);
    std::printf("Accumulator ticks after large dt: %d\n", engine.getAccumulatorTicks());

    std::printf("\n=== Demo complete! ===\n");

    return 0;
}