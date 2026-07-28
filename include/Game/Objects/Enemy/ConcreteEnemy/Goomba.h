#pragma once
#include "../Enemy.h"

class Goomba : public Enemy {
public:
    Goomba();
    Goomba(sf::Texture& texture, const std::string& animationSetId = "goomba");
    ~Goomba() override = default;

protected:
    void updateSimulation(const float &fixedDt) override;
private:
    float _moveSpeed = 5.0f;
};