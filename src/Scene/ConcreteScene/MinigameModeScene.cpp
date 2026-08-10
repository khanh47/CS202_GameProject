#include "Scene/ConcreteScene/MinigameModeScene.h"
#include "Commands/FunctionalCommand.h"
#include "Game/GameSettings.h"
#include "ResourceManager.h"
#include "Scene/ConcreteScene/InGameScene.h"
#include "Scene/SceneManager.h"

MinigameModeScene::MinigameModeScene(const std::string& mapPath)
    : Scene("MinigameModeScene"),
      _mapPath(mapPath),
      _titleText(ResourceManager::getInstance().getFont("Roboto"), "SELECT MINIGAME MODE", 64),
      _comingSoonText(ResourceManager::getInstance().getFont("Roboto"), "VS AI - Coming Soon", 40) {
    sf::FloatRect bounds = _titleText.getLocalBounds();
    _titleText.setOrigin({bounds.position.x + bounds.size.x / 2.0f,
                          bounds.position.y + bounds.size.y / 2.0f});
    _titleText.setPosition({960.0f, 200.0f});
    _titleText.setFillColor(sf::Color::Black);

    sf::FloatRect soonBounds = _comingSoonText.getLocalBounds();
    _comingSoonText.setOrigin({soonBounds.position.x + soonBounds.size.x / 2.0f,
                               soonBounds.position.y + soonBounds.size.y / 2.0f});
    _comingSoonText.setPosition({960.0f, 540.0f});
    _comingSoonText.setFillColor(sf::Color::Red);

    setBackground("main_menu_background");
}

void MinigameModeScene::onEnter() {
    Scene::onEnter();
    _setupButtons();
}

void MinigameModeScene::handleInput(const sf::Event& event) {
    if (_showComingSoon) {
        _showComingSoon = false;
        return;
    }

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

void MinigameModeScene::render(sf::RenderTarget& target) {
    Scene::render(target);
    target.draw(_titleText);
    _buttonMenu.render(target);
    if (_showComingSoon) {
        target.draw(_comingSoonText);
    }
}

void MinigameModeScene::_setupButtons() {
    _buttonMenu.clear();
    _buttonMenu.setLayoutProperties(
        {820.0f, 300.0f},
        {280.0f, 60.0f},
        75.0f,
        false,
        sf::Color(100, 149, 237),
        28
    );

    _buttonMenu.addButtonAuto("2 Player", std::make_unique<FunctionalCommand>(
        "2 Player", [this]() {
            GameSettings::getInstance().minigameMode = MinigameMode::TwoPlayer;
            std::string modeDir = "2p";
            if (GameSettings::getInstance().minigameMode == MinigameMode::VsAi) {
                modeDir = "vsai";
            }
            if (auto mgr = getSceneManager()) {
                mgr->pushScene(std::make_unique<InGameScene>(
                    "assets/datas/minigames/" + modeDir + "/" + _mapPath + ".json"));
            }
        }
    ));

    _buttonMenu.addButtonAuto("VS AI", std::make_unique<FunctionalCommand>(
        "VS AI", [this]() {
            GameSettings::getInstance().minigameMode = MinigameMode::VsAi;
            _showComingSoon = true;
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
