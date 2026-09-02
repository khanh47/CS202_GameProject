#include "Game/Behaviours/ShellHoldBehaviour.h"

#include <cmath>
#include <memory>

#include "Game/Behaviours/Animatable.h"
#include "Game/Objects/GameObject.h"
#include "Game/Objects/Player/Player.h"
#include "Game/Objects/Projectile/KoopaShell.h"
#include "Game/World/GameWorld.h"

namespace {
bool aabbOverlap(
    const sf::Vector2f& aPos,
    const sf::Vector2f& aSize,
    const sf::Vector2f& bPos,
    const sf::Vector2f& bSize
) {
    return std::abs(aPos.x - bPos.x) < (aSize.x + bSize.x) * 0.5f
        && std::abs(aPos.y - bPos.y) < (aSize.y + bSize.y) * 0.5f;
}
}

void ShellHoldBehaviour::updateSimulation(const float& fixedDt) {
    (void)fixedDt;

    auto* owner = getOwner();
    if (!owner || !owner->getPhysicsBody() || !owner->getPhysicsBody()->isValid()) {
        return;
    }

    if (_heldShell) {
        if (_heldShell->isPendingDestroy()
            || _heldShell->isDying()
            || !_heldShell->getPhysicsBody()
            || !_heldShell->getPhysicsBody()->isValid()) {
            releaseShell(false);
            return;
        }

        auto* player = dynamic_cast<Player*>(owner);
        const bool facingRight = !(player && player->isFacingLeft());
        const float facing = facingRight ? 1.0f : -1.0f;
        const float forwardOffset =
            owner->getHitboxPixels().x * 0.45f
            + _heldShell->getHitboxPixels().x * 0.4f;
        const sf::Vector2f holdPos = {
            owner->getPosition().x + facing * forwardOffset,
            owner->getPosition().y + 4.0f
        };
        _heldShell->setPosition(holdPos);
        _heldShell->setFacingRight(facingRight);
        return;
    }

    const bool shiftHeld = _interactHeld
        || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)
        || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift);

    if (shiftHeld) {
        tryPickUpShell();
    }
}

void ShellHoldBehaviour::updateVisuals(float deltaTime) {
    (void)deltaTime;

    if (!_heldShell) {
        return;
    }

    auto* owner = getOwner();
    if (!owner || !owner->getPhysicsBody() || !owner->getPhysicsBody()->isValid()) {
        releaseShell(false);
        return;
    }

    if (_heldShell->isPendingDestroy()
        || _heldShell->isDying()
        || !_heldShell->getPhysicsBody()
        || !_heldShell->getPhysicsBody()->isValid()) {
        releaseShell(false);
        return;
    }

    // Pin the shell in front of the player's hands at waist/torso height
    auto* player = dynamic_cast<Player*>(owner);
    const bool facingRight = !(player && player->isFacingLeft());
    const float facing = facingRight ? 1.0f : -1.0f;
    const float forwardOffset =
        owner->getHitboxPixels().x * 0.45f
        + _heldShell->getHitboxPixels().x * 0.4f;
    const sf::Vector2f holdPos = {
        owner->getPosition().x + facing * forwardOffset,
        owner->getPosition().y + 4.0f
    };
    _heldShell->setPosition(holdPos);
    _heldShell->setFacingRight(facingRight);
}

void ShellHoldBehaviour::tryPickUpShell() {
    auto* owner = getOwner();
    auto* player = dynamic_cast<Player*>(owner);
    if (!player) {
        return;
    }

    GameWorld* world = player->getGameWorld();
    if (!world) {
        return;
    }

    const sf::Vector2f playerPos = owner->getPosition();
    const sf::Vector2f playerSize = owner->getHitboxPixels();
    const sf::Vector2f querySize = {playerSize.x + 20.0f, playerSize.y};

    KoopaShell* best = nullptr;
    float bestDistance = 0.0f;
    for (const std::shared_ptr<GameObject>& object : world->objects()) {
        if (!object) {
            continue;
        }

        auto* shell = dynamic_cast<KoopaShell*>(object.get());
        if (!shell
            || shell->isHeld()
            || shell->isSliding()
            || shell->isDying()
            || shell->isPendingDestroy()
            || !shell->getPhysicsBody()
            || !shell->getPhysicsBody()->isValid()) {
            continue;
        }

        const sf::Vector2f shellPos = shell->getPosition();
        if (!aabbOverlap(playerPos, querySize, shellPos, shell->getHitboxPixels())) {
            continue;
        }

        const float dx = shellPos.x - playerPos.x;
        const float dy = shellPos.y - playerPos.y;
        const float distance = dx * dx + dy * dy;
        if (!best || distance < bestDistance) {
            best = shell;
            bestDistance = distance;
        }
    }

    if (best) {
        holdShell(best);
    }
}

void ShellHoldBehaviour::holdShell(KoopaShell* shell) {
    if (!shell) {
        return;
    }
    _heldShell = shell;
    _heldShell->setHeld(true);
    _heldShell->resetReviveTimer();
    _heldShell->stop();

    auto* owner = getOwner();
    if (owner) {
        auto* player = dynamic_cast<Player*>(owner);
        const bool facingRight = !(player && player->isFacingLeft());
        const float facing = facingRight ? 1.0f : -1.0f;
        const float forwardOffset =
            owner->getHitboxPixels().x * 0.45f
            + _heldShell->getHitboxPixels().x * 0.4f;
        _heldShell->setPosition({
            owner->getPosition().x + facing * forwardOffset,
            owner->getPosition().y + 4.0f
        });
        _heldShell->setFacingRight(facingRight);
    }
}

bool ShellHoldBehaviour::tryHoldContact(KoopaShell& shell) {
    if (_heldShell) {
        return false;
    }
    const bool shiftHeld = _interactHeld
        || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)
        || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift);
    if (!shiftHeld) {
        return false;
    }
    if (shell.isSliding() || shell.isDying() || shell.isPendingDestroy()) {
        return false;
    }
    holdShell(&shell);
    return true;
}

void ShellHoldBehaviour::releaseShell(bool throwAway) {
    if (!_heldShell) {
        return;
    }

    KoopaShell* shell = _heldShell;
    _heldShell = nullptr;

    if (!shell->isPendingDestroy()
        && !shell->isDying()
        && shell->getPhysicsBody()
        && shell->getPhysicsBody()->isValid()) {
        shell->setHeld(false);
        if (throwAway) {
            auto* player = dynamic_cast<Player*>(getOwner());
            const bool facingRight = !(player && player->isFacingLeft());
            if (player) {
                // Offset the throw ahead of the player so the sliding shell
                // never spawns overlapping their body.
                const float facing = facingRight ? 1.0f : -1.0f;
                const float clearDistance =
                    player->getHitboxPixels().x * 0.5f
                    + shell->getHitboxPixels().x * 0.5f
                    + 10.0f;
                const sf::Vector2f pos = player->getPosition();
                shell->setPosition({pos.x + facing * clearDistance, pos.y + 4.0f});
            }
            shell->kick(facingRight);
            if (player) {
                if (auto* animatable = player->getBehaviour<Animatable>()) {
                    animatable->playAnimation("throw", true);
                }
            }
        }
    }
}

void ShellHoldBehaviour::setInteractHeld(bool held) {
    if (_interactHeld == held) {
        return;
    }
    _interactHeld = held;
    if (!held && _heldShell) {
        releaseShell(true);
    }
}

void ShellHoldBehaviour::onDetach() {
    releaseShell(false);
}
