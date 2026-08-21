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
    void setPipeTravel(float hiddenYPixels, float emergedYPixels);
    void setPipeTravel(
        sf::Vector2f hiddenPosition,
        sf::Vector2f emergedPosition
    );

protected:
    void onCreateBodyDef(b2BodyDef& def) override;
    void onCreateShapeDef(b2ShapeDef& def) override;

private:
    enum class PipePhase {
        Hidden,
        Rising,
        Exposed,
        Retracting
    };

    void beginPhase(PipePhase phase);

    bool _hasPipeTravel = false;
    PipePhase _pipePhase = PipePhase::Hidden;
    float _phaseTimer = 0.0f;
    sf::Vector2f _hiddenPosition{};
    sf::Vector2f _emergedPosition{};
};
