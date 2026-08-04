#pragma once

#include <box2d/box2d.h>
#include <SFML/System.hpp>
#include <memory>

#include "Game/Behaviours/Animatable.h"
#include "Game/Behaviours/Damageable.h"
#include "Game/Objects/GameObject.h"

class TerrainSeamFilter;

class Enemy : public GameObject {
public:
    Enemy();
    Enemy(sf::Texture& texture);
    Enemy(sf::Texture &texture, const std::string& animationSetId);
    ~Enemy();
    void onContact(GameObject& other, const b2ContactData& contactData, b2ShapeId ownShape) override;
    void setSupportGrid(const TerrainSeamFilter* filter, float cellSize = 64.0f);

protected:
    std::unique_ptr<Animatable> animatable;
    std::unique_ptr<Damageable> damageable;

    virtual void onCreateBodyDef(b2BodyDef& def) override;
    virtual void onCreateShapeDef(b2ShapeDef& def) override;
    virtual void onUpdateVisuals(float deltaTime) override;
    virtual void onRenderVisual(sf::RenderTarget& target, const sf::Vector2f& position, float angleDegrees) override;
    virtual void updateSimulation(const float &fixedDt) override;

    struct SensorProbeResult {
        bool touching = false;
        int overlapCount = 0;
        b2AABB worldAABB = {};
    };
    SensorProbeResult probeSensor() const;
private:
    bool isSupportedByGrid(int& outCol, int& outRow) const;

    float _moveSpeed = 3.0f;
    int _unsupportedSteps = 0;
    int _simTick = 0;
    bool _wasTouching = false;
    const TerrainSeamFilter* _supportGrid = nullptr;
    float _supportCellSize = 64.0f;
    static constexpr int unsupportedStepsBeforeTurn = 6;
};
