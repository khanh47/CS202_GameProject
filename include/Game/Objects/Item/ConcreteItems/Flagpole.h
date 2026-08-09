#pragma once

#include "../Item.h"

class Flagpole : public Item {
public:
    Flagpole();
    Flagpole(sf::Texture& texture);
    ~Flagpole() override = default;

    void onContact(GameObject& other, const b2ContactData& contactData, b2ShapeId ownShape) override;

protected:
    void onCreateBodyDef(b2BodyDef& def) override;
    void onCreateShapeDef(b2ShapeDef& def) override;

private:
    void configureVisuals(sf::Texture& texture);

    bool _triggered = false;
};
