#include "Scene/ConcreteScene/LevelSelectionScene.h"
#include <filesystem>

#include "Commands/FunctionalCommand.h"
#include "ResourceManager.h"
#include "Scene/ConcreteScene/InGameScene.h"
#include "Scene/SceneManager.h"

LevelSelectionScene::LevelSelectionScene()
    : Scene("LevelSelectionScene"),
      _titleText(ResourceManager::getInstance().getFont("SuperMario"), "SELECT LEVEL", 64) {
    sf::FloatRect bounds = _titleText.getLocalBounds();
    _titleText.setOutlineThickness(5.0f);
    _titleText.setOutlineColor(sf::Color::Black);
    _titleText.setFillColor(sf::Color::White);
    _titleText.setOrigin({bounds.position.x + bounds.size.x / 2.0f,
                          bounds.position.y + bounds.size.y / 2.0f});
    _titleText.setPosition({960.0f, 220.0f});
    _titleText.setFillColor(sf::Color::White);
}

void LevelSelectionScene::onEnter() {
    Scene::onEnter();
    _setupButtons();
}

void LevelSelectionScene::handleInput(const sf::Event& event) {
    if (auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
        if (keyEvent->code == sf::Keyboard::Key::Escape) {
            if (auto mgr = getSceneManager()) {
                mgr->requestPopScene();
                return;
            }
        }
    }

    _buttonMenu.processEvent(event);
}

void LevelSelectionScene::render(sf::RenderTarget& target) {
    Scene::render(target);
    target.draw(_titleText);
    _buttonMenu.render(target);
}

void LevelSelectionScene::_setupButtons() {
    _buttonMenu.clear();
    _buttonMenu.setLayoutProperties(
        {820.0f, 320.0f},
        {280.0f, 60.0f},
        75.0f,
        false,
        sf::Color(100, 149, 237),
        28
    );

    const std::string base = "assets/datas/levels/map-";

    _buttonMenu.addButtonAuto("Level 1", std::make_unique<FunctionalCommand>(
        "Level 1", [this, base]() {
            if (auto mgr = getSceneManager()) {
                mgr->pushScene(std::make_unique<InGameScene>(base + "1.json"));
            }
        }
    ));

    _buttonMenu.addButtonAuto("Level 2", std::make_unique<FunctionalCommand>(
        "Level 2", [this, base]() {
            if (auto mgr = getSceneManager()) {
                mgr->pushScene(std::make_unique<InGameScene>(base + "2.json"));
            }
        }
    ));

    _buttonMenu.addButtonAuto("Level 3", std::make_unique<FunctionalCommand>(
        "Level 3", [this, base]() {
            if (auto mgr = getSceneManager()) {
                mgr->pushScene(std::make_unique<InGameScene>(base + "3.json"));
            }
        }
    ));

    const std::string customMap = "assets/datas/levels/custom-map.json";
    if (std::filesystem::exists(customMap)) {
        _buttonMenu.addButtonAuto("Custom Map", std::make_unique<FunctionalCommand>(
            "Custom Map", [this, customMap]() {
                if (auto mgr = getSceneManager()) {
                    mgr->pushScene(std::make_unique<InGameScene>(customMap));
                }
            }
        ));
    }
}
