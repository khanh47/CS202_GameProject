#pragma once

#include "../Item.h"

class Player;

/**
 * @brief Large Mega Mushroom pickup that grants the temporary Mega state.
 */
class MegaMushroom : public Item {
public:
    MegaMushroom();
    MegaMushroom(sf::Texture& texture);
    ~MegaMushroom() override = default;

    void onPickup(Player& player);

protected:
    void onCreateBodyDef(b2BodyDef& def) override;
    void onCreateShapeDef(b2ShapeDef& def) override;
    void updateSimulation(const float& fixedDt) override;

private:
    bool _movingRight = true;
    static constexpr float _speed = 3.0f;
};
