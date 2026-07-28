#include "Scene/ConcreteScene/SettingsScene.h"
#include "ResourceManager.h"
#include "Scene/SceneManager.h"
#include "Commands/ToggleDebugCommands.h"
#include "Game/GameSettings.h"

SettingsScene::SettingsScene()
    : _name("SettingsScene") {
}

void SettingsScene::init() {
    _menu.setLayoutProperties({1920.f / 2.f - 150.f, 400.f}, {300.f, 60.f}, 80.f, false, sf::Color(100, 149, 237), 24);
    
    auto& settings = GameSettings::getInstance();

    _menu.addToggleButtonAuto("Grid", settings.debugDrawGrid, std::make_unique<ToggleGridCommand>());
    _menu.addToggleButtonAuto("Coordinates", settings.debugDrawCoordinates, std::make_unique<ToggleCoordinatesCommand>());
    _menu.addToggleButtonAuto("Camera Move", settings.freeCameraMove, std::make_unique<ToggleFreeCameraCommand>());
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
                mgr->requestPopScene();
                return;
            }
        }
    }
    _menu.processEvent(event);
}

void SettingsScene::updateVisuals(float deltaTime) {
    _menu.updateVisuals(deltaTime);
}

void SettingsScene::render(sf::RenderTarget& target) {
    const sf::Font& font = ResourceManager::getInstance().getFont("Roboto");
    sf::Text text(font, "Settings scene", 48);
    text.setFillColor(sf::Color::Black);
    text.setPosition({1920.f / 2.f - text.getLocalBounds().size.x / 2.f, 200.f});
    target.draw(text);
    
    _menu.render(target);
}
