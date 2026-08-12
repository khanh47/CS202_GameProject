#pragma once

#include <SFML/Window/Event.hpp>
#include <memory>
#include <vector>

class GameObject;
class PlayerController;

class WorldObjectStore {
public:
    WorldObjectStore();
    ~WorldObjectStore();

    void clear();
    void addObject(std::shared_ptr<GameObject> object);
    void addController(std::unique_ptr<PlayerController> controller);

    bool handleInput(const sf::Event& event);
    void updateSimulation(float fixedDt);
    void finalizeSimulation(float fixedDt);
    void updateVisuals(float deltaTime);
    void cleanupDestroyed();
    void suspendPlayerMotion();
    void finalizeGroundContacts();
    void syncControllersWithKeyboard();
    void sortOut();

    std::shared_ptr<GameObject> getPrimaryPlayer() const;
    bool hasLivingPlayers() const;
    const std::vector<std::shared_ptr<GameObject>>& objects() const noexcept {
        return _objects;
    }
    std::vector<std::shared_ptr<GameObject>>& objects() noexcept {
        return _objects;
    }

private:
    std::vector<std::shared_ptr<GameObject>> _objects;
    std::vector<std::unique_ptr<PlayerController>> _controllers;
};
