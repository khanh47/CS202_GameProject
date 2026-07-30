#pragma once

#include <box2d/box2d.h>
#include <SFML/System.hpp>
#include <string>
#include <memory>

#include "Game/Behaviours/Animatable.h"
#include "Game/Behaviours/Damageable.h"
#include "Game/Behaviours/Moveable.h"
#include "Game/Objects/GameObject.h"
#include "Game/Objects/Player/State/PlayerState.h"

class GameWorld;

class Player: public GameObject,
              public Animatable,
              public Damageable, 
              public Moveable {
public:
    Player();
    Player(sf::Texture &texture);
    Player(sf::Texture &texture, const std::string& animationSetId);
    ~Player() override;

    void setState(std::unique_ptr<PlayerState> newState);
    void changeToNormalState();
    void changeToSuperState();
    void changeToFireState();
    void applyMegaState(float durationSeconds = 5.0f);
    void revertDecoratedState();

    void attack(GameWorld& world);
    void startFireTransformation(GameWorld& world, float duration = 1.0f);

    void setGameWorld(GameWorld& world) { _world = &world; }
    void onContact(GameObject& other) override;

    PlayerState* getState() const { return _state.get(); }
    bool isTransforming() const { return _isTransforming; }

protected:
    void updateSimulation(const float &fixedDt) override;
    void finalizeSimulation(const float &fixedDt) override;
    void onCreateBodyDef(b2BodyDef& def) override;
    void onCreateShapeDef(b2ShapeDef& def) override;
    void onUpdateVisuals(float deltaTime) override;
    void onRenderVisual(sf::RenderTarget& target, const sf::Vector2f& position, float angleDegrees) override;
    void onHitboxRecreated() override;
    b2Polygon makeHitbox(sf::Vector2f hitboxPixels) const override;

private:
    float _baseMoveSpeed = 8.0f;
    float _baseJumpSpeed = 16.0f;
    std::unique_ptr<PlayerState> _state;
    std::unique_ptr<IAttackStrategy> _attackStrategy;

    bool _isTransforming = false;
    float _transformTimer = 0.0f;
    float _transformDuration = 1.0f;
    GameWorld* _world = nullptr;
};
