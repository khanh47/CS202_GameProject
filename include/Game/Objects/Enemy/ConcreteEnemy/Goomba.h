#pragma once
#include "../Enemy.h"

// Goomba inherits core physical properties from Enemy (which inherits GameObject + Animatable + Damageable)
class Goomba : public Enemy {
public:
    Goomba();
    Goomba(sf::Texture& texture);
    ~Goomba() override = default;

protected:
    // Customize Box2D body configurations
    void onCreateBodyDef(b2BodyDef& def) override;
    void onCreateShapeDef(b2ShapeDef& def) override;
};