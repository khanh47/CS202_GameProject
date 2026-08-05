#pragma once

#include <box2d/box2d.h>
#include <SFML/System.hpp>
#include <string>
#include <memory>

#include "Game/Behaviours/Moveable.h"
#include "Game/Behaviours/SparkleEffect.h"
#include "Game/Objects/GameObject.h"
#include "Game/Objects/Player/State/PlayerState.h"

class GameWorld;

class Player: public GameObject {
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
    void applyStarManState(float durationSeconds = 10.0f);
    void revertDecoratedState();

    enum class TransformTarget { Super, Fire, StarMan, None };

    void attack(GameWorld& world);
    void startTransformation(TransformTarget target, GameWorld& world, float duration = 1.0f);
    void startTransformation(TransformTarget target, float duration = 1.0f);

    void setGameWorld(GameWorld& world) { _world = &world; }
    const std::string& getCharacter() const { return _character; }
    void onContact(GameObject& other, const b2ContactData& contactData, b2ShapeId ownShape) override;
    void finalizeGroundContacts() override;

    PlayerState* getState() const { return _state.get(); }
    bool isTransforming() const { return _isTransforming; }

    // Moveable forwarding (called externally)
    bool isFacingLeft() const {
        if (const auto* moveable = getBehaviour<Moveable>()) {
            return moveable->isFacingLeft();
        }
        return false;
    }
    void startMoveLeft() {
        if (auto* moveable = getBehaviour<Moveable>()) moveable->startMoveLeft();
    }
    void startMoveRight() {
        if (auto* moveable = getBehaviour<Moveable>()) moveable->startMoveRight();
    }
    void startJump() {
        if (auto* moveable = getBehaviour<Moveable>()) moveable->startJump();
    }
    void stopMoveLeft() {
        if (auto* moveable = getBehaviour<Moveable>()) moveable->stopMoveLeft();
    }
    void stopMoveRight() {
        if (auto* moveable = getBehaviour<Moveable>()) moveable->stopMoveRight();
    }
    void stopJump() {
        if (auto* moveable = getBehaviour<Moveable>()) moveable->stopJump();
    }
    void beginGroundContact(b2ShapeId visitor) {
        if (auto* moveable = getBehaviour<Moveable>()) moveable->beginGroundContact(visitor);
    }
    void endGroundContact(b2ShapeId visitor) {
        if (auto* moveable = getBehaviour<Moveable>()) moveable->endGroundContact(visitor);
    }

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
    std::string _character = "mario";

    bool _isTransforming = false;
    float _transformTimer = 0.0f;
    float _transformDuration = 1.0f;
    float _transformStartScale = 1.0f;
    TransformTarget _transformTarget = TransformTarget::Fire;
    SparkleEffect _starSparkle{30.0f, 0.5f};
    float _effectTime = 0.0f;
    GameWorld* _world = nullptr;
};
