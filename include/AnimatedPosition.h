#pragma once
#include <cmath>

class AnimatedPosition {
public:
    static constexpr float ANIMATION_SPEED = 3.0f;
    static constexpr float SPAWN_DURATION = 0.6f;

    void setTarget(float x, float y);
    void startSpawn();
    void update(float dt);
    void snapTo(float x, float y);

    [[nodiscard]] float getCurrentX() const;
    [[nodiscard]] float getCurrentY() const;
    [[nodiscard]] float getCurrentScale() const;

private:
    float currentX = 0;
    float currentY = 0;
    float targetX = 0;
    float targetY = 0;
    float progress = 1.0f;

    bool isSpawning = false;
    float spawnTimer = 0.0f;
    float currentScale = 1.0f;
};
