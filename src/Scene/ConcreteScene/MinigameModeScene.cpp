#include "Scene/ConcreteScene/MinigameModeScene.h"
#include "Commands/FunctionalCommand.h"
#include "Game/GameSettings.h"
#include "Game/AI/AiGenomeCodec.h"
#include "ResourceManager.h"
#include "Scene/ConcreteScene/InGameScene.h"
#include "Scene/SceneManager.h"

#include <exception>

MinigameModeScene::MinigameModeScene(const std::string& mapPath)
    : Scene("MinigameModeScene"),
      _mapPath(mapPath),
      _titleText(ResourceManager::getInstance().getFont("SuperMario"), "SELECT MINIGAME MODE", 64),
      _statusText(ResourceManager::getInstance().getFont("SuperMario"), "", 32) {

    _titleText.setOutlineThickness(5.0f);
    _titleText.setOutlineColor(sf::Color::Black);
    _titleText.setFillColor(sf::Color::White);

    sf::FloatRect bounds = _titleText.getLocalBounds();
    _titleText.setOrigin({bounds.position.x + bounds.size.x / 2.0f,
                          bounds.position.y + bounds.size.y / 2.0f});
    _titleText.setPosition({960.0f, 200.0f});
    _titleText.setFillColor(sf::Color::White
    );

    _statusText.setPosition({960.0f, 570.0f});
    _statusText.setFillColor(sf::Color::Red);
    _statusText.setOutlineColor(sf::Color::Black);
    _statusText.setOutlineThickness(2.0f);
}

void MinigameModeScene::onEnter() {
    Scene::onEnter();
    _showStatus = false;
    _setupButtons();
}

void MinigameModeScene::handleInput(const sf::Event& event) {
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
    if (_showStatus) {
        const sf::FloatRect bounds = _statusText.getLocalBounds();
        _statusText.setOrigin({
            bounds.position.x + bounds.size.x * 0.5f,
            bounds.position.y + bounds.size.y * 0.5f
        });
        target.draw(_statusText);
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

            if (auto mgr = getSceneManager()) {
                mgr->pushScene(std::make_unique<InGameScene>(
                    "assets/datas/minigames/" + modeDir + "/" + _mapPath + ".json"));
            }
        }
    ));

    _buttonMenu.addButtonAuto("VS AI", std::make_unique<FunctionalCommand>(
        "VS AI", [this]() {
            GameSettings::getInstance().minigameMode = MinigameMode::VsAi;
            std::string modeDir = "ai";
            try {
                (void)AiGenomeCodec::load(
                    "assets/ai/" + _mapPath + ".json"
                );
                _showStatus = false;
                if (auto mgr = getSceneManager()) {
                    mgr->pushScene(std::make_unique<InGameScene>(
                        "assets/datas/minigames/" + modeDir + "/"
                        + _mapPath + ".json"
                    ));
                }
            } catch (const std::exception& error) {
                _statusText.setString(
                    "AI model unavailable. Run MarioAiTrainer.\n"
                    + std::string(error.what())
                );
                _showStatus = true;
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
