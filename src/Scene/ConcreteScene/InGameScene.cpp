#include "InGameScene.h"
#include "ResourceManager.h"
#include "SceneManager.h"

InGameScene::InGameScene(const std::string& name)
    : _name(name) {
}

void InGameScene::init() {
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
            }
        }
    }
}

void InGameScene::update(float deltaTime) {
    (void)deltaTime;
}

void InGameScene::render(sf::RenderTarget& target) {
    (void)target;
    const sf::Font& font = ResourceManager::getInstance().getFont("Roboto");
    sf::Text text(font, "InGame scene", 24);
    text.setFillColor(sf::Color::Black);
    target.draw(text);
}
