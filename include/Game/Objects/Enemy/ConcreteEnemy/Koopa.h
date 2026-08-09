#pragma once
#include "../Enemy.h"

class GameWorld;

class Koopa : public Enemy {
public:
    Koopa();
    Koopa(sf::Texture& texture, const std::string& animationSetId = "koopa");
    ~Koopa() override = default;
    void onStomp() override;
    void updateSimulation(const float &fixedDt) override;

    void setGameWorld(GameWorld* world) { _world = world; }

private:
    void onUpdateVisuals(float deltaTime) override;

    GameWorld* _world = nullptr;
};
