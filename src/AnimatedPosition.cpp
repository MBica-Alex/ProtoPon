#include "AnimatedPosition.h"
#include <cmath>

namespace {
    float easeOutCubic(float t) {
        return 1.0f - std::pow(1.0f - t, 3.0f);
    }

    float lerp(float start, float end, float t) {
        return start + (end - start) * t;
    }
}

void AnimatedPosition::setTarget(float x, float y) {
    if (std::abs(targetX - x) > 0.1f || std::abs(targetY - y) > 0.1f) {
        targetX = x;
        targetY = y;
        progress = 0.0f;
    }
}

void AnimatedPosition::startSpawn() {
    isSpawning = true;
    spawnTimer = 0.0f;
    currentScale = 0.0f;
}

void AnimatedPosition::update(float dt) {
    if (progress < 1.0f) {
        progress += dt * ANIMATION_SPEED;
        if (progress > 1.0f) progress = 1.0f;
        float eased = easeOutCubic(progress);
        currentX = lerp(currentX + (targetX - currentX) * (1.0f - eased), targetX, eased);
        currentY = lerp(currentY + (targetY - currentY) * (1.0f - eased), targetY, eased);
    } else {
        currentX = targetX;
        currentY = targetY;
    }

    if (isSpawning) {
        spawnTimer += dt;
        float t = spawnTimer / SPAWN_DURATION;
        if (t >= 1.0f) {
            isSpawning = false;
            currentScale = 1.0f;
        } else {
            const float c1 = 3.0f; 
            const float c3 = c1 + 1.0f;
            currentScale = 1.0f + c3 * std::pow(t - 1.0f, 3.0f) + c1 * std::pow(t - 1.0f, 2.0f);
        }
    }
}

void AnimatedPosition::snapTo(float x, float y) {
    currentX = targetX = x;
    currentY = targetY = y;
    progress = 1.0f;
}

float AnimatedPosition::getCurrentX() const { return currentX; }
float AnimatedPosition::getCurrentY() const { return currentY; }
float AnimatedPosition::getCurrentScale() const { return currentScale; }
