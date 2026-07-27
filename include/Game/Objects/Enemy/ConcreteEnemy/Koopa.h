#pragma once
#include "../Enemy.h"

class Koopa : public Enemy {
public:
    Koopa();
    Koopa(sf::Texture& texture, const std::string& animationSetId = "koopa");
    ~Koopa() override = default;

protected:
    void onUpdateVisuals(float deltaTime) override;
    void updateSimulation(const float &fixedDt) override;
private:
    float _moveSpeed = 5.0f;
};