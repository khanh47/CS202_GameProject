#include "Scene/ConcreteScene/SettingsScene.h"
#include "ResourceManager.h"
#include "Scene/SceneManager.h"
#include "Commands/ToggleDebugCommands.h"
#include "Commands/FunctionalCommand.h"
#include "Game/GameSettings.h"

SettingsScene::SettingsScene()
    : Scene("SettingsScene") {
    setBackground("main_menu_background");
}

void SettingsScene::init() {
    _menu.setLayoutProperties({1920.f / 2.f - 170.f, 280.f}, {340.f, 60.f}, 75.f, false, sf::Color(100, 149, 237), 24);
    
    auto& settings = GameSettings::getInstance();

    _menu.addToggleButtonAuto("Grid", settings.debugDrawGrid, std::make_unique<ToggleGridCommand>());
    _menu.addToggleButtonAuto("Coordinates", settings.debugDrawCoordinates, std::make_unique<ToggleCoordinatesCommand>());
    _menu.addToggleButtonAuto("Hitbox", settings.debugDrawHitbox, std::make_unique<ToggleHitboxCommand>());
    _menu.addToggleButtonAuto("Camera Move", settings.freeCameraMove, std::make_unique<ToggleFreeCameraCommand>());

    _menu.addButtonAuto("Keybind Settings", std::make_unique<FunctionalCommand>(
        "KeybindSettings", [this]() {
            if (auto mgr = getSceneManager()) {
                mgr->pushSceneByName("KEYBIND_SETTINGS");
            }
        }
    ));

    _menu.addButtonAuto("Back", 24, std::make_unique<FunctionalCommand>(
        "BackToMain", [this]() {
            if (auto mgr = getSceneManager()) {
                mgr->requestPopScene();
            }
        }
    ), sf::Color(180, 80, 80));
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
    Scene::render(target);

    const sf::Font& font = ResourceManager::getInstance().getFont("Roboto");
    sf::Text text(font, "SETTINGS", 48);
    text.setFillColor(sf::Color::Black);
    text.setStyle(sf::Text::Bold);
    text.setPosition({1920.f / 2.f - text.getLocalBounds().size.x / 2.f, 160.f});
    target.draw(text);
    
    _menu.render(target);
}
