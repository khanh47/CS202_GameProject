#include "Scene/ConcreteScene/LevelSelectionScene.h"
#include "Commands/FunctionalCommand.h"
#include "Game/GameSettings.h"
#include "ResourceManager.h"
#include "Scene/ConcreteScene/InGameScene.h"
#include "Scene/SceneManager.h"

LevelSelectionScene::LevelSelectionScene()
    : _name("LevelSelectionScene"),
      _titleText(ResourceManager::getInstance().getFont("Roboto"), "SELECT LEVEL", 64) {
    sf::FloatRect bounds = _titleText.getLocalBounds();
    _titleText.setOrigin({bounds.position.x + bounds.size.x / 2.0f,
                          bounds.position.y + bounds.size.y / 2.0f});
    _titleText.setPosition({960.0f, 220.0f});
    _titleText.setFillColor(sf::Color::Black);
}

void LevelSelectionScene::init() {
}

void LevelSelectionScene::onEnter() {
    _isActive = true;
    _setupButtons();
}

void LevelSelectionScene::onExit() {
    _isActive = false;
}

void LevelSelectionScene::cleanup() {
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

void LevelSelectionScene::updateVisuals(float deltaTime) {
    (void)deltaTime;
}

void LevelSelectionScene::render(sf::RenderTarget& target) {
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

    const GameSettings& settings = GameSettings::getInstance();
    const std::string modeDir =
        settings.gameMode == GameMode::Solo
            ? "solo-" + settings.player1Character
            : "coop";
    const std::string base = "assets/datas/levels/" + modeDir + "/level-";

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
}
