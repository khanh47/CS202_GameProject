#pragma once
#include <SFML/Graphics.hpp>
#include <string>

class SceneManager;

class Scene {
public:
    virtual ~Scene() = default;

    // Lifecycle
    virtual void init() = 0;
    virtual void onEnter() = 0;
    virtual void onExit() = 0;
    virtual void cleanup() = 0;

    // Per-frame
    virtual void handleInput(const sf::Event& event) = 0;
    virtual void update(float deltaTime) = 0;
    virtual void render(sf::RenderTarget& target) = 0;

    // Queries
    virtual std::string getName() const = 0;
    virtual bool isActive() const = 0;

    // Scene manager access (for scene transitions)
    void setSceneManager(SceneManager* manager) { _sceneManager = manager; }
    SceneManager* getSceneManager() const { return _sceneManager; }

protected:
    bool _isActive = false;
    SceneManager* _sceneManager = nullptr;
};