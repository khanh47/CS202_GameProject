#include "Game/Objects/Block/Block.h"
#include "Game/Behaviours/ShellHoldBehaviour.h"
#include "Audio/SoundManager.h"
#include "Game/Objects/Player/Player.h"
#include "Game/Behaviours/Animatable.h"
#include "Game/World/GameWorld.h"

Block::Block() : GameObject() {
    addBehaviour<Animatable>();
}

Block::Block(sf::Texture &texture) : Block() {
    if (auto* animatable = getBehaviour<Animatable>()) {
        animatable->configureVisuals(texture);
    }
}

void Block::onContact(GameObject& other, const b2ContactData& contactData, b2ShapeId ownShape) {
    if (tryBreakOnContact(other, contactData, ownShape)) {
        return;
    }
    isBumped(other, contactData, ownShape);
}

Block::~Block() {
}

void Block::spawnBreakEffect(GameWorld& world) const {
    const sf::Texture* texture = nullptr;
    sf::IntRect textureRect;
    if (auto* animatable = getBehaviour<Animatable>();
        animatable && animatable->hasSprite()) {
        texture = animatable->getTexture();
        textureRect = animatable->getTextureRect();
    } else {
        texture = _breakTexture;
        textureRect = _breakTextureRect;
    }

    world.spawnBlockBreakEffect(
        getPosition(),
        _hitboxPixels,
        texture,
        textureRect
    );
}

void Block::onCreateShapeDef(b2ShapeDef& def) {
    def.density = 10000.0f;
    def.material.friction = 0.0f;
}

void Block::onUpdateVisuals(float deltaTime) {
    if (auto* animatable = getBehaviour<Animatable>()) {
        animatable->updateVisualState(deltaTime, _hitboxPixels);
    }
}

void Block::onRenderVisual(sf::RenderTarget& target, const sf::Vector2f& position, float angleDegrees) {
    if (auto* animatable = getBehaviour<Animatable>()) {
        animatable->renderVisualState(target, position, angleDegrees);
    }
}

bool Block::tryBreakOnContact(
    GameObject& other,
    const b2ContactData& contactData,
    b2ShapeId ownShape
) {
    if (!_breakable || isPendingDestroy()) {
        return false;
    }

    auto* player = dynamic_cast<Player*>(&other);
    if (!player
        || !b2Shape_IsValid(ownShape)
        || contactData.manifold.pointCount <= 0) {
        return false;
    }

    b2Vec2 blockToPlayer = contactData.manifold.normal;
    if (!B2_ID_EQUALS(contactData.shapeIdA, ownShape)) {
        blockToPlayer = {-blockToPlayer.x, -blockToPlayer.y};
    }

    // A positive Y normal points from the block toward a player underneath it.
    const bool hitFromBelow = blockToPlayer.y >= 0.5f;
    // A negative Y normal is a player landing on top of the block.
    const bool highFallLanding = blockToPlayer.y <= -0.5f
        && player->hasFallenFromHighPlace();
    if (!hitFromBelow && !highFallLanding) {
        return false;
    }

    GameWorld* world = player->getGameWorld();
    if (world) {
        Audio::SoundManager::getInstance().playEffect("break");
        spawnBreakEffect(*world);

        if (world->getScoreManager()) {
            world->getScoreManager()->handleEvent(
                ScoreEventType::BlockBroken,
                getPosition()
            );
        }
    }
    destroy();
    return true;
}

bool Block::isBumped(GameObject& other, const b2ContactData& contactData, b2ShapeId ownShape) {
    if (auto* player = dynamic_cast<Player*>(&other)) {
        if (b2Shape_IsValid(ownShape) && contactData.manifold.pointCount > 0) {
            b2Vec2 normal = contactData.manifold.normal;
            if (!B2_ID_EQUALS(contactData.shapeIdA, ownShape)) {
                normal = {-normal.x, -normal.y};
            }

            // Detect Player hitting from below (upward contact normal or player below block)
            if (normal.y >= 0.3f || player->getPosition().y > getPosition().y) {
                auto* playerAnimatable = player->getBehaviour<Animatable>();
                auto* holdingShell = player->getBehaviour<ShellHoldBehaviour>();
                // Mega-state players are too large to react with a bump animation.
                if (playerAnimatable &&
                    (!holdingShell || !holdingShell->isHoldingShell()) &&
                    !player->isMegaState()) {
                    playerAnimatable->playAnimation("bump", true);
                }
                return true;
            }
            else return false;
        }
    }
    return false;
}
