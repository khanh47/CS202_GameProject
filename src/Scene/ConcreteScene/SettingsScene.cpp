#include "SettingsScene.h"
#include "ResourceManager.h"
#include "SceneManager.h"

SettingsScene::SettingsScene()
    : _name("SettingsScene") {
}

void SettingsScene::init() {
}

void SettingsScene::onEnter() {
    _isActive = true;
}

void SettingsScene::onExit() {
    _isActive = false;
}

void SettingsScene::cleanup() {
}

void SettingsScene::handleInput(const sf::Event& event) {
    if (auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
        if (keyEvent->code == sf::Keyboard::Key::Escape) {
            if (auto mgr = getSceneManager()) {
                mgr->popScene();
            }
        }
    }
}

void SettingsScene::update(float deltaTime) {
    (void)deltaTime;
}

void SettingsScene::render(sf::RenderTarget& target) {
    (void)target;
    const sf::Font& font = ResourceManager::getInstance().getFont("Roboto");
    sf::Text text(font, "Settings scene", 24);
    text.setFillColor(sf::Color::Black);
    target.draw(text);
}
