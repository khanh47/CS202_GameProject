#pragma once

#include <box2d/box2d.h>
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>

#include "Game/Objects/GameObject.h"
#include "Physics/PhysicsWorld.h"
#include "Game/Behaviours/Animatable.h"

// Animated coin that pops out of the block when hit
struct BouncingCoin {
    sf::Vector2f position;
    float startY = 0.0f;
    float velocityY = -350.0f;
    float gravity = 1200.0f;
    bool active = false;
    Animatable animatable;

    void spawn(sf::Vector2f spawnPos, sf::Texture& texture) {
        position = spawnPos;
        position.y -= 64.0f; // Start above the block
        startY = spawnPos.y - 64.0f;
        velocityY = -360.0f;
        active = true;
        animatable.configureVisuals(texture, "coin");
        animatable.playAnimation("idle");
    }

    void update(float deltaTime) {
        if (!active) return;
        position.y += velocityY * deltaTime;
        velocityY += gravity * deltaTime;
        animatable.updateVisualState(deltaTime, {64.0f, 64.0f});
        if (position.y >= startY && velocityY > 0) {
            active = false;
        }
    }

    void render(sf::RenderTarget& target) const {
        if (!active) return;
        // Cast away constness for rendering animatable
        const_cast<Animatable&>(animatable).renderVisualState(target, position);
    }
};

class CoinBlock: public GameObject {
public:
    CoinBlock();
    CoinBlock(sf::Texture &texture);
    ~CoinBlock() override;

    void onContact(GameObject& other, const b2ContactData& contactData, b2ShapeId ownShape) override;

protected:
    void onCreateShapeDef(b2ShapeDef& def) override;
    void onUpdateVisuals(float deltaTime) override;
    void onRenderVisual(sf::RenderTarget& target, const sf::Vector2f& position, float angleDegrees) override;

private:
    int capacity = 10;          // Number of coins the block can give
    float _hitCooldown = 0.0f;  // Cooldown time after being hit
    float _bumpTimer = 0.0f;    // Bumping/enlarging visual effect timer
    BouncingCoin _bouncingCoin; // Animated coin popping out
};
