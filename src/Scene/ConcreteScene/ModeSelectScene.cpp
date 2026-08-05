#include "Scene/ConcreteScene/ModeSelectScene.h"
#include "Commands/FunctionalCommand.h"
#include "Game/GameSettings.h"
#include "ResourceManager.h"
#include "Scene/ConcreteScene/MinigameModeScene.h"
#include "Scene/SceneManager.h"

ModeSelectScene::ModeSelectScene()
    : _name("ModeSelectScene"),
      _titleText(ResourceManager::getInstance().getFont("Roboto"), "CHOOSE GAME MODE", 64) {
    sf::FloatRect bounds = _titleText.getLocalBounds();
    _titleText.setOrigin({bounds.position.x + bounds.size.x / 2.0f,
                          bounds.position.y + bounds.size.y / 2.0f});
    _titleText.setPosition({960.0f, 220.0f});
    _titleText.setFillColor(sf::Color::Black);
}

void ModeSelectScene::init() {
}

void ModeSelectScene::onEnter() {
    _isActive = true;
    _setupButtons();
}

void ModeSelectScene::onExit() {
    _isActive = false;
}

void ModeSelectScene::cleanup() {
}

void ModeSelectScene::handleInput(const sf::Event& event) {
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

void ModeSelectScene::updateVisuals(float deltaTime) {
    (void)deltaTime;
}

void ModeSelectScene::render(sf::RenderTarget& target) {
    target.draw(_titleText);
    _buttonMenu.render(target);
}

void ModeSelectScene::_setupButtons() {
    _buttonMenu.clear();
    _buttonMenu.setLayoutProperties(
        {820.0f, 320.0f},
        {280.0f, 60.0f},
        75.0f,
        false,
        sf::Color(100, 149, 237),
        28
    );

    _buttonMenu.addButtonAuto("Solo", std::make_unique<FunctionalCommand>(
        "Solo", [this]() {
            GameSettings::getInstance().gameMode = GameMode::Solo;
            if (auto mgr = getSceneManager()) {
                mgr->pushSceneByName("CHARACTER_SELECT");
            }
        }
    ));

    _buttonMenu.addButtonAuto("Coop", std::make_unique<FunctionalCommand>(
        "Coop", [this]() {
            GameSettings::getInstance().gameMode = GameMode::Coop;
            if (auto mgr = getSceneManager()) {
                mgr->pushSceneByName("LEVEL_SELECT");
            }
        }
    ));

    _buttonMenu.addButtonAuto("Minigames", std::make_unique<FunctionalCommand>(
        "Minigames", [this]() {
            GameSettings::getInstance().gameMode = GameMode::Minigame;
            if (auto mgr = getSceneManager()) {
                mgr->pushScene(std::make_unique<MinigameModeScene>("minigame-1"));
            }
        }
    ));

    _buttonMenu.addButtonAuto("Back", std::make_unique<FunctionalCommand>(
        "Back", [this]() {
            if (auto mgr = getSceneManager()) {
                mgr->requestPopScene();
            }
        }
    ));
}
