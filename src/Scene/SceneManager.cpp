#include "Scene/SceneManager.h"
#include "Scene/Scene.h"
#include "Scene/SceneFactory.h"
#include <iostream>

SceneManager::SceneManager(SceneFactory *factory)
    : _factory(factory) {
}

SceneManager::~SceneManager() {
    while (!isEmpty()) {
        auto &top = _sceneStack.top();
        top->onExit();
        top->cleanup();
        _sceneStack.pop();
    }
}

void SceneManager::pushScene(std::unique_ptr<Scene> scene) {
    if (!scene) return;

    scene->setSceneManager(this);
    if (!_sceneStack.empty()) {
        _sceneStack.top()->onExit();
    }
    _sceneStack.push(std::move(scene));
    _sceneStack.top()->init();
    _sceneStack.top()->onEnter();
    std::cout << "Pushed scene: " << _sceneStack.top()->getName() << std::endl;
}

void SceneManager::popScene() {
    if (!_sceneStack.empty()) {
        auto &top = _sceneStack.top();
        top->onExit();
        top->cleanup();
        _sceneStack.pop();
        if (!_sceneStack.empty()) {
            _sceneStack.top()->onEnter();
        }
    }
}

void SceneManager::replaceScene(std::unique_ptr<Scene> scene) {
    if (!scene) return;

    if (!_sceneStack.empty()) {
        auto &top = _sceneStack.top();
        top->onExit();
        top->cleanup();
        _sceneStack.pop();
    }
    _sceneStack.push(std::move(scene));
    _sceneStack.top()->init();
    _sceneStack.top()->onEnter();
    std::cout << "Replaced scene with: " << _sceneStack.top()->getName() << std::endl;
}

void SceneManager::pushSceneByName(const std::string &sceneName) {
    if (!_factory) {
        std::cerr << "Error: SceneFactory not set" << std::endl;
        return;
    }
    auto scene = _factory->createScene(sceneName);
    if (scene) {
        pushScene(std::move(scene));
    }
}

void SceneManager::processEvents(const sf::Event& event) {
    if (!_sceneStack.empty()) {
        auto &currentScene = _sceneStack.top();
        if (currentScene->isActive()) {
            currentScene->handleInput(event);
        }
    }
}

void SceneManager::update(float deltaTime) {
    if (!_sceneStack.empty()) {
        auto &currentScene = _sceneStack.top();
        if (currentScene->isActive()) {
            currentScene->update(deltaTime);
        }
    }
}

void SceneManager::render(sf::RenderTarget& target) {
    if (!_sceneStack.empty()) {
        auto &currentScene = _sceneStack.top();
        if (currentScene->isActive()) {
            currentScene->render(target);
        }
    } else {
        std::cerr << "Warning: No scene to render." << std::endl;
    }
}

Scene *SceneManager::getCurrentScene() const {
    if (!isEmpty()) {
        return _sceneStack.top().get();
    }
    return nullptr;
}

bool SceneManager::isEmpty() const {
    return _sceneStack.empty();
}

void SceneManager::processDeferredActions() {
    while (!_deferredActions.empty()) {
        _deferredActions.front()();
        _deferredActions.pop();
    }
}

