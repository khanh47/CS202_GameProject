#pragma once
#include "../Enemy.h"

class PiranhaPlant : public Enemy {
public:
    PiranhaPlant();
    PiranhaPlant(sf::Texture& texture, const std::string& animationSetId = "piranha_plant");
    ~PiranhaPlant() override = default;
    void updateSimulation(const float &fixedDt) override;
    void onStomp() override;
    bool canBeStomped() const override;
};
