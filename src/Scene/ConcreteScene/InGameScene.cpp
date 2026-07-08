#include <memory>

#include "Scene/ConcreteScene/InGameScene.h"
#include "ResourceManager.h"
#include "Scene/SceneManager.h"

InGameScene::InGameScene(const std::string& name)
    : _name(name) {
}

void InGameScene::init() {
    _gameWorld.test();
    _camera.setCenter({1920.f / 2.f, _gameWorld.getGridHeight() * _gameWorld.getCellSize() - 1080.f / 2.f});
}

void InGameScene::onEnter() {
    _isActive = true;
}

void InGameScene::onExit() {
    _isActive = false;
}

void InGameScene::cleanup() {
}

void InGameScene::handleInput(const sf::Event& event) {
    if (auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
        if (keyEvent->code == sf::Keyboard::Key::Escape) {
            if (auto mgr = getSceneManager()) {
                mgr->popScene();
                return;
            }
        }
    }

    _gameWorld.handleInput(event);
}

void InGameScene::updateSimulation(const float &fixedDt) {
    _gameWorld.updateSimulation(fixedDt);
}

void InGameScene::updateVisuals(float deltaTime) {
    _camera.update(deltaTime);
}

void InGameScene::render(sf::RenderTarget& target) {
    sf::View defaultView = target.getDefaultView();
    target.setView(_camera.getView());

    _gameWorld.render(target);

    target.setView(defaultView);

    const sf::Font& font = ResourceManager::getInstance().getFont("Roboto");
    sf::Text text(font, "InGame scene", 24);
    text.setFillColor(sf::Color::Black);
    target.draw(text);
}
