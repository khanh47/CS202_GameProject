#include "Game/World/WorldObjectStore.h"
#include "Game/Objects/GameObject.h"
#include "Game/Objects/Player/Player.h"
#include "Game/UserInput/PlayerController.h"
#include <memory>

WorldObjectStore::WorldObjectStore() = default;
WorldObjectStore::~WorldObjectStore() = default;

void WorldObjectStore::clear() {
    _controllers.clear();
    _objects.clear();
}

void WorldObjectStore::addObject(std::shared_ptr<GameObject> object) {
    if (object) {
        _objects.push_back(std::move(object));
    }
}

void WorldObjectStore::addController(
    std::unique_ptr<PlayerController> controller
) {
    if (controller) {
        _controllers.push_back(std::move(controller));
    }
}

bool WorldObjectStore::handleInput(const sf::Event& event) {
    bool handled = false;
    for (const std::unique_ptr<PlayerController>& controller : _controllers) {
        if (controller) {
            handled = controller->handleEvent(event) || handled;
        }
    }
    return handled;
}

void WorldObjectStore::updateSimulation(float fixedDt) {
    for (const std::shared_ptr<GameObject>& object : _objects) {
        if (object) {
            object->updateSimulation(fixedDt);
        }
    }
}

void WorldObjectStore::finalizeSimulation(float fixedDt) {
    finalizeGroundContacts();
    for (const std::shared_ptr<GameObject>& object : _objects) {
        if (object) {
            object->finalizeSimulation(fixedDt);
        }
    }
}

void WorldObjectStore::updateVisuals(float deltaTime) {
    for (const std::shared_ptr<GameObject>& object : _objects) {
        if (object) {
            object->updateVisuals(deltaTime);
        }
    }
}

void WorldObjectStore::cleanupDestroyed() {
    std::erase_if(_controllers, [](const std::unique_ptr<PlayerController>& controller) {
        return !controller || controller->isPlayerPendingDestroy();
    });
    std::erase_if(_objects, [](const std::shared_ptr<GameObject>& object) {
        return !object || object->isPendingDestroy();
    });
}

void WorldObjectStore::suspendPlayerMotion() {
    for (const std::shared_ptr<GameObject>& object : _objects) {
        if (const auto player = std::dynamic_pointer_cast<Player>(object)) {
            player->stopMoveLeft();
            player->stopMoveRight();
            player->stopJump();
        }
    }
}

void WorldObjectStore::finalizeGroundContacts() {
    for (const std::shared_ptr<GameObject>& object : _objects) {
        if (object) {
            object->finalizeGroundContacts();
        }
    }
}

void WorldObjectStore::syncControllersWithKeyboard() {
    for (const std::unique_ptr<PlayerController>& controller : _controllers) {
        if (controller) {
            controller->syncStateWithKeyboard();
        }
    }
}

std::shared_ptr<GameObject> WorldObjectStore::getPrimaryPlayer() const {
    for (const std::shared_ptr<GameObject>& object : _objects) {
        if (std::dynamic_pointer_cast<Player>(object)) {
            return object;
        }
    }
    return nullptr;
}

bool WorldObjectStore::hasLivingPlayers() const {
    for (const std::shared_ptr<GameObject>& object : _objects) {
        const auto player = std::dynamic_pointer_cast<Player>(object);
        if (player) {
            return true;
        }
    }
    return false;
}