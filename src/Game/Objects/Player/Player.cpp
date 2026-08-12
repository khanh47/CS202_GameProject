#include "Game/Objects/Player/Player.h"
#include "Game/GameSettings.h"
#include "Game/Objects/GameObject.h"
#include "Game/World/GameWorld.h"
#include "Game/Objects/Player/State/NormalState.h"
#include "Game/Objects/Player/State/SuperState.h"
#include "Game/Objects/Player/State/FireState.h"
#include "Game/Objects/Player/State/MegaStateDecorator.h"
#include "Game/Objects/Player/State/StarManStateDecorator.h"
#include "Physics/CollisionFilter.h"
#include "Physics/PhysicsUnits.h"
#include "Game/Objects/Block/CoinBlock.h"
#include "Game/Objects/Enemy/Enemy.h"
#include "Game/Objects/Item/ConcreteItems/FireFlower.h"
#include "Game/Objects/Item/ConcreteItems/SuperMushroom.h"
#include "Game/Objects/Item/ConcreteItems/SuperStar.h"
#include "Game/Objects/Item/ConcreteItems/Coin.h"
#include "Game/Objects/Item/ConcreteItems/MegaCoin.h"
#include "Game/Objects/Projectile/KoopaShell.h"
#include "Game/Objects/Pipe/Pipe.h"
#include "ResourceManager.h"
#include "Game/Objects/Player/PlayerShaders.h"
#include "Game/Behaviours/Invincible.h"
#include "box2d/id.h"

#include <SFML/System/Clock.hpp>
#include <algorithm>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Vector2.hpp>
#include <ctime>
#include <iostream>
#include <iterator>

Player::Player() : GameObject() {
    addBehaviour<Animatable>();
    addBehaviour<Damageable>(100);
    addBehaviour<Moveable>();
    addBehaviour<ShellHoldBehaviour>();
    setState(std::make_unique<NormalState>());
}

Player::Player(sf::Texture &texture) : Player(texture, "mario") {
}

Player::Player(sf::Texture &texture, const std::string& animationSetId)
    : GameObject() {
    (void)texture;
    addBehaviour<Animatable>();
    addBehaviour<Damageable>(100);
    addBehaviour<Moveable>();
    addBehaviour<ShellHoldBehaviour>();
    _character = animationSetId.find("luigi") != std::string::npos ? "luigi" : "mario";
    if (animationSetId.rfind("fire_", 0) == 0) {
        setState(std::make_unique<FireState>(_character));
    } else {
        setState(std::make_unique<NormalState>(_character));
    }
}

Player::~Player() = default;

void Player::destroy() {
    if (auto* invincible = getBehaviour<Invincible>()) return;

    if (auto* hold = getBehaviour<ShellHoldBehaviour>()) {
        hold->releaseShell(false);
    }

    b2ShapeId shape = _body->getHitbox();
    b2Filter filter = b2Shape_GetFilter(shape);
    filter.maskBits ^= CollisionFilter::ENEMY | CollisionFilter::SHELL;
    if (GameSettings::getInstance().gameMode == GameMode::Minigame) {
      filter.maskBits ^= CollisionFilter::MINIGAME_MASK;
    }
    b2Shape_SetFilter(shape, filter);

    if (_isDying) {
        return;
    }

    _isDying = true;

    removeBehaviour<Moveable>();
    auto* animatable = getBehaviour<Animatable>();
    if (animatable) {
        animatable->playAnimation("knockout");
    }
    // _pendingDestroy is set later in updateSimulation once knockout finishes.
}

void Player::finalizeGroundContacts() {
    auto* moveable = getBehaviour<Moveable>();
    if (!moveable) {
        return;
    }

    moveable->finalizeGroundContacts();
    // Reset the stomp combo whenever the player is on solid ground,
    // so the ladder only grows during genuine airborne chains.
    if (!moveable->isAirbone()) {
        if (_world && _world->getScoreManager()) {
            _world->getScoreManager()->handleEvent(ScoreEventType::MarioLanded);
        }
    }
}

void Player::setState(std::unique_ptr<PlayerState> newState) {
    if (!newState) return;

    if (_state) {
        _state->onExit(*this);
    }

    _state = std::move(newState);
    _state->onEnter(*this);
    _attackStrategy = _state->createAttackStrategy();

    // Apply texture and animation set dynamically from the current state
    const std::string& texAlias = _state->getTextureAlias();
    const std::string& animId = _state->getAnimationSetId();
    sf::Texture& tex = ResourceManager::getInstance().getTexture(texAlias);
    if (auto* animatable = getBehaviour<Animatable>()) {
        animatable->configureVisuals(tex, animId);
    }
}

void Player::attack(GameWorld& world) {
    // Can't shoot fireballs while carrying a shell.
    if (auto* hold = getBehaviour<ShellHoldBehaviour>()) {
        if (hold->isHoldingShell()) {
            return;
        }
    }
    if (_state) {
        _attackStrategy = _state->createAttackStrategy();
    }
    if (_attackStrategy) {
        _attackStrategy->executeAttack(*this, world);
    }
}

void Player::changeToNormalState() {
    setState(std::make_unique<NormalState>(_character));
}

void Player::changeToSuperState() {
    setState(std::make_unique<SuperState>(_character));
}

void Player::changeToFireState() {
    setState(std::make_unique<FireState>(_character));
}

void Player::applyMegaState(float durationSeconds) {
    if (!_state) {
        _state = std::make_unique<NormalState>(_character);
    }
    setState(std::make_unique<MegaStateDecorator>(std::move(_state), durationSeconds));
}

void Player::applyStarManState(float durationSeconds) {
    if (!_state) {
        _state = std::make_unique<NormalState>(_character);
    }
    auto* starDecorator = dynamic_cast<StarManStateDecorator*>(_state.get());
    if (starDecorator) {
        starDecorator->resetTimer(durationSeconds);
        return;
    }
    setState(std::make_unique<StarManStateDecorator>(std::move(_state), durationSeconds));
}

void Player::revertDecoratedState() {
    if (!_state) return;

    auto* decorator = dynamic_cast<PlayerStateDecorator*>(_state.get());
    if (decorator) {
        std::unique_ptr<PlayerState> unwrapped = decorator->unwrap();
        if (unwrapped) {
            setState(std::move(unwrapped));
        }
    }
}

void Player::updateSimulation(const float &fixedDt) {
    if (!_body || !_body->isValid()) {
        return;
    }

    _warpCooldown = std::max(0.0f, _warpCooldown - fixedDt);
    if (_world && _warpCooldown <= 0.0f
        && (_interactHeld || _moveDownHeld || _moveUpHeld)) {
        if (_world->tryWarpPlayer(*this)) {
            _warpCooldown = 0.35f;
            return;
        }
    }

    b2Vec2 velocity = b2Body_GetLinearVelocity(_body->getId());

    auto* invincible = getBehaviour<Invincible>();
    auto* animatable = getBehaviour<Animatable>();
    auto* moveable = getBehaviour<Moveable>();

    
    if (invincible) {
        invincible->updateSimulation(fixedDt);
        if(invincible->getTime() == 0.0){
            removeBehaviour<Invincible>();
            velocity.y -= 2.01;
        };
    }
    

    if (_isDying) {
        if (!animatable) {
            _pendingDestroy = true;
            return;
        }

        if (animatable->getActiveAnimationName() != "knockout") {
            animatable->playAnimation("knockout");
        }

        if (animatable->isAnimationDone()) {
            _pendingDestroy = true;
        }
        return;
    }

    if (!moveable) {
        return;
    }

    if (auto* hold = getBehaviour<ShellHoldBehaviour>()) {
        hold->updateSimulation(fixedDt);
    }

    float moveSpeed = _baseMoveSpeed;
    float jumpSpeed = _baseJumpSpeed;
    if (_state) {
        moveSpeed *= _state->getMoveSpeedMultiplier();
        jumpSpeed *= _state->getJumpSpeedMultiplier();
    }

    if (moveable->isMovingLeft() && !moveable->isMovingRight()) {
        velocity.x = -moveSpeed;
    } else if (moveable->isMovingRight() && !moveable->isMovingLeft()) {
        velocity.x = moveSpeed;
    } else if (!moveable->isMovingLeft() && !moveable->isMovingRight()) {
        velocity.x = 0.f;
    }

    if (moveable->isJumping() && !moveable->isAirbone()) {
        velocity.y = -jumpSpeed;
        moveable->consumeGroundForJump();
    }


    b2Body_SetGravityScale(_body->getId(), 4.0f);
    if (moveable->isAirbone() || moveable->isJumping()) {
        if (velocity.y > 0) b2Body_SetGravityScale(_body->getId(), moveable->isJumping() ? 3.0f : 4.0f);
    }

    b2Body_SetLinearVelocity(_body->getId(), velocity);
}

void Player::finalizeSimulation(const float &fixedDt) {
    (void)fixedDt;

    auto* moveable = getBehaviour<Moveable>();
    auto* animatable = getBehaviour<Animatable>();
    if (!moveable || !animatable) {
        return;
    }

    if(!animatable->isLooping() && !animatable->isAnimationDone()) return;

    const auto* hold = getBehaviour<ShellHoldBehaviour>();
    const bool holding = hold && hold->isHoldingShell();
    if (holding) {
        if (!moveable->isMovingLeft() && !moveable->isMovingRight()) {
            animatable->playAnimation("hold_stand");
        } else {
            animatable->playAnimation("hold_walk");
        }
    } else if (moveable->isAirbone() || moveable->isJumping()) {
        animatable->playAnimation("jump");
    } else if (!moveable->isMovingLeft() && !moveable->isMovingRight()) {
        animatable->playAnimation("idle");
    } else {
        animatable->playAnimation("walk");
    }
}

void Player::onContact(GameObject& other, const b2ContactData& contactData, b2ShapeId ownShape) {
    if (auto* mushroom = dynamic_cast<SuperMushroom*>(&other)) {
        if (_state) {
            _state->handleSuperMushroom(*this);
        }

        if (_world && _world->getScoreManager()) {
            _world->getScoreManager()->handleEvent(ScoreEventType::PowerupCollected, getPosition());
        }

        mushroom->destroy();
        return;
    }

    if (auto* fireFlower = dynamic_cast<FireFlower*>(&other)) {
        if (_state) {
            _state->handleFireFlower(*this);
        }

        if (_world && _world->getScoreManager()) {
            _world->getScoreManager()->handleEvent(ScoreEventType::PowerupCollected, getPosition());
        }

        fireFlower->destroy();
        return;
    }

    if (auto* star = dynamic_cast<SuperStar*>(&other)) {
        if (_state) {
            _state->handleSuperStar(*this);
        }

        if (_world && _world->getScoreManager()) {
            _world->getScoreManager()->handleEvent(ScoreEventType::PowerupCollected, getPosition());
        }

        star->destroy();
        return;
    }

    if (auto* coin = dynamic_cast<Coin*>(&other)) {
        if (_world) {
            //_world->incrementScore(100);
        }

        if (_world && _world->getScoreManager()) {
            _world->getScoreManager()->handleEvent(ScoreEventType::CoinCollected, getPosition());
        }

        coin->destroy();
        return;
    }

    if (auto* megaCoin = dynamic_cast<MegaCoin*>(&other)) {
        if (_world) {
            //_world->incrementScore(100);
        }

        if (_world && _world->getScoreManager()) {
            _world->getScoreManager()->handleEvent(ScoreEventType::MegaCoinCollected, getPosition());
        }

        megaCoin->destroy();
        return;
    }

    if (auto* shell = dynamic_cast<KoopaShell*>(&other)) {
        // A shell held by this player must not be kicked or damage us.
        if (shell->isHeld()) {
            return;
        }

        // StarMan invincibility: instantly destroy any shell on contact
        if (_state && _state->isInvincible()) {

            if (_world && _world->getScoreManager()) {
                _world->getScoreManager()->handleEvent(ScoreEventType::EnemyStomped, shell->getPosition());
            }

            shell->destroy();
            return;
        }

        if (b2Shape_IsValid(ownShape)) {
            b2Vec2 normal = contactData.manifold.normal;
            if (!B2_ID_EQUALS(contactData.shapeIdA, ownShape)) {
                normal = {-normal.x, -normal.y};
            }
            if (contactData.manifold.pointCount > 0 && normal.y >= 0.5f) {
                // Stomping the shell: stop a sliding shell, kick a resting one
                if (shell->isSliding()) {
                    shell->stop();
                } else {
                    shell->kick(!isFacingLeft());
                }
                b2BodyId bodyId = b2Shape_GetBody(ownShape);
                b2Vec2 vel = b2Body_GetLinearVelocity(bodyId);
                vel.y = -12.0f;
                b2Body_SetLinearVelocity(bodyId, vel);
            } else {
                // Side contact: sliding shells hurt, resting shells get kicked
                if (shell->isSliding() && _state) {
                    _state->handleEnemy(*this);
                } else {
                    shell->kick(!isFacingLeft());
                }
            }
        }
        return;
    }

    if (auto* enemy = dynamic_cast<Enemy*>(&other)) {
        // StarMan invincibility: instantly destroy any enemy on contact
        if (_state && _state->isInvincible()) {

            if (_world && _world->getScoreManager()) {
                _world->getScoreManager()->handleEvent(ScoreEventType::EnemyStomped, enemy->getPosition());
            }

            enemy->destroy();
            return;
        }

        if (b2Shape_IsValid(ownShape)) {
            b2Vec2 normal = contactData.manifold.normal;
            if (!B2_ID_EQUALS(contactData.shapeIdA, ownShape)) {
                normal = {-normal.x, -normal.y};
            }
            if (contactData.manifold.pointCount > 0 && normal.y >= 0.5f && enemy->canBeStomped()) {
                if (_world && _world->getScoreManager()) {
                    // Triggers 100 -> 200 -> 400 -> 800 -> 1000 -> 2000 -> 4000 -> 8000 -> 1UP
                    _world->getScoreManager()->handleEvent(ScoreEventType::EnemyStomped, enemy->getPosition());
                }

                enemy->onStomp();
                b2BodyId bodyId = b2Shape_GetBody(ownShape);
                b2Vec2 vel = b2Body_GetLinearVelocity(bodyId);
                vel.y = -12.0f;
                b2Body_SetLinearVelocity(bodyId, vel);
            } else {
                _state->handleEnemy(*this);
            }
        }
    }

    if (auto* player = dynamic_cast<Player*>(&other)) {
        if (GameSettings::getInstance().gameMode != GameMode::Minigame) {
            return;
        }
        if (b2Shape_IsValid(ownShape)) {
            b2Vec2 normal = contactData.manifold.normal;
            if (!B2_ID_EQUALS(contactData.shapeIdA, ownShape)) {
                normal = {-normal.x, -normal.y};
            }
            if (contactData.manifold.pointCount > 0 && normal.y >= 0.5f) {
                player->destroy();
                b2BodyId bodyId = b2Shape_GetBody(ownShape);
                b2Vec2 vel = b2Body_GetLinearVelocity(bodyId);
                vel.y = -12.0f;
                b2Body_SetLinearVelocity(bodyId, vel);
            } else {
                if (auto* damageable = getBehaviour<Damageable>()) {
                    damageable->takeDamage(50);
                }
            }
        }
        return;
    }
}

void Player::onCreateBodyDef(b2BodyDef& def) {
    def.type = b2_dynamicBody;
    def.motionLocks.angularZ = true;
}

void Player::onCreateShapeDef(b2ShapeDef& def) {
    def.density = 1.0f;
    def.material.friction = 0.0f;
    def.enablePreSolveEvents = true;

    def.filter.categoryBits = CollisionFilter::PLAYER;
    def.filter.maskBits = CollisionFilter::PLAYER_MASK;
    if (GameSettings::getInstance().gameMode == GameMode::Minigame)
        def.filter.maskBits |= CollisionFilter::MINIGAME_MASK;
}

b2Polygon Player::makeHitbox(sf::Vector2f hitboxPixels) const {
    constexpr float cornerRadiusPixels = 4.0f;
    const float halfWidthPixels =
        std::max(0.0f, hitboxPixels.x * 0.5f - cornerRadiusPixels);
    const float halfHeightPixels =
        std::max(0.0f, hitboxPixels.y * 0.5f - cornerRadiusPixels);
    return b2MakeRoundedBox(
        PhysicsUnits::toMeters(halfWidthPixels),
        PhysicsUnits::toMeters(halfHeightPixels),
        PhysicsUnits::toMeters(cornerRadiusPixels)
    );
}

void Player::startTransformation(TransformTarget target, float duration) {
    if (_world) {
        startTransformation(target, *_world, duration);
    }
}

void Player::startTransformation(TransformTarget target, GameWorld& world, float duration) {
    if (_isTransforming) return;

    _isTransforming = true;
    _transformTimer = duration;
    _transformDuration = duration > 0.0f ? duration : 1.0f;
    _transformTarget = target;

    // Snapshot current scale so the animation lerps from it to the target (1.25x)
    _transformStartScale = _state ? _state->getScaleMultiplier().x : 1.0f;

    // Stop movement inputs and clear horizontal physics velocity when transformation starts
    if (auto* moveable = getBehaviour<Moveable>()) {
        moveable->stopMoveLeft();
        moveable->stopMoveRight();
        moveable->stopJump();
    }
    if (hasValidBody()) {
        const b2BodyId bodyId = _body->getId();
        b2Vec2 vel = b2Body_GetLinearVelocity(bodyId);
        vel.x = 0.0f;
        b2Body_SetLinearVelocity(bodyId, vel);
    }

    // Freeze physics simulation & world updates for the duration of transformation
    world.freeze(duration);

    // The blink shader handles the visual flash effect, so no spritesheet swap needed.
    // Reset effect clock so the blink starts cleanly from t=0.
    _effectTime = 0.0f;
}

void Player::onUpdateVisuals(float deltaTime) {
    _effectTime += deltaTime;
    PlayerShaders::getInstance().update(deltaTime);
    auto* moveable = getBehaviour<Moveable>();
    const bool facingLeft = moveable ? moveable->isFacingLeft() : false;

    if (auto* hold = getBehaviour<ShellHoldBehaviour>()) {
        hold->updateVisuals(deltaTime);
    }

    if (_isTransforming) {
        _transformTimer -= deltaTime;
        if (_transformTimer <= 0.0f) {
            _isTransforming = false;
            if (auto* moveable = getBehaviour<Moveable>()) {
                moveable->stopMoveLeft();
                moveable->stopMoveRight();
                moveable->stopJump();
            }
            if (hasValidBody()) {
                const b2BodyId bodyId = _body->getId();
                b2Vec2 vel = b2Body_GetLinearVelocity(bodyId);
                vel.x = 0.0f;
                b2Body_SetLinearVelocity(bodyId, vel);
            }
            // Transformation finished -> enter the correct target state
            if (_transformTarget == TransformTarget::Normal) {
                changeToNormalState();
            }
            else if (_transformTarget == TransformTarget::Fire) {
                changeToFireState();
            } else if (_transformTarget == TransformTarget::StarMan) {
                applyStarManState(10.0f);
            } else if (_transformTarget == TransformTarget::Super) {
                changeToSuperState();
            } else if (_transformTarget == TransformTarget::None) {
                if (_state) {
                    sf::Texture& tex = ResourceManager::getInstance().getTexture(_state->getTextureAlias());
                    if (auto* animatable = getBehaviour<Animatable>()) {
                        animatable->configureVisuals(tex, _state->getAnimationSetId());
                    }
                }
            }
            return;
        }

        // Lerp from the pre-transformation scale to target scale
        float targetScale = (_transformTarget == TransformTarget::StarMan) ? _transformStartScale : 1.5f;
        float progress = 1.0f - (_transformTimer / _transformDuration);
        progress = std::max(0.0f, std::min(1.0f, progress));
        float currentScale = _transformStartScale + (targetScale - _transformStartScale) * progress;

        sf::Vector2f scaledHitbox = {_baseHitboxPixels.x * currentScale, _baseHitboxPixels.y * currentScale};
        updateHitboxSize(scaledHitbox);
        if (auto* animatable = getBehaviour<Animatable>()) {
            sf::Vector2f scale;
            if(animatable->getActiveAnimationName() == "shoot") scale = {3.75f, 1.1f};
            else scale = {2.5f, 1.1f};
            animatable->updateVisualState(deltaTime, scaledHitbox, facingLeft);
        }
        return;
    }

    sf::Vector2f scaleMult{1.0f, 1.0f};
    if (_state) {
        _state->update(*this, deltaTime);
        // State replacement is deferred until update() returns, so a decorator
        // never destroys itself while one of its member functions is active.
        if (_state->isExpired()) {
            revertDecoratedState();
        }
        if (_state) {
            scaleMult = _state->getScaleMultiplier();
        }
    }

    sf::Vector2f scaledHitbox = {_baseHitboxPixels.x * scaleMult.x, _baseHitboxPixels.y * scaleMult.y};
    updateHitboxSize(scaledHitbox);
    if (auto* animatable = getBehaviour<Animatable>()) {
        sf::Vector2f scale;
        if(animatable->getActiveAnimationName() == "shoot") scale = {3.75f, 1.1f};
        else scale = {2.5f, 1.1f};
        animatable->setVisualScale(scale);
        animatable->updateVisualState(deltaTime, scaledHitbox, facingLeft);
    }

    // Drive sparkle particles when in invincible (StarMan) state
    if (_state && _state->isInvincible() && hasValidBody()) {
        sf::Vector2f pos = getPosition();
        _starSparkle.update(deltaTime, pos, {scaledHitbox.x * 0.5f, scaledHitbox.y * 0.5f});
    }
}

void Player::onRenderVisual(sf::RenderTarget& target, const sf::Vector2f& position, float angleDegrees) {
    (void)angleDegrees;

    // Select the appropriate shader effect for the current visual state
    sf::Shader* activeShader = nullptr;

    
    
    if (_isTransforming) {
        activeShader = PlayerShaders::getInstance().getBlinkShader();
    } else if (_state && _state->isInvincible()) {
        activeShader = PlayerShaders::getInstance().getRainbowShader();
    }
    else if (auto* tempInvincible = getBehaviour<Invincible>()) {
        activeShader = PlayerShaders::getInstance().getGhostShader();
    }

    if (auto* animatable = getBehaviour<Animatable>()) {
        animatable->renderVisualState(target, position, 0.0f, activeShader);
    }

    // Overlay sparkle particles during StarMan invincibility
    if (_state && _state->isInvincible()) {
        _starSparkle.render(target);
    }
}

void Player::onHitboxRecreated() {
    if (auto* moveable = getBehaviour<Moveable>()) {
        moveable->resetGroundContacts();
    }

    if (auto* invincible = getBehaviour<Invincible>()) {
        invincible->refreshCollisionMask();
    }
}
