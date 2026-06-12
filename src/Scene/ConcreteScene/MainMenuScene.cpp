#include "Scene/ConcreteScene/MainMenuScene.h"
#include "Commands/FunctionalCommand.h"
#include "ResourceManager.h"
#include "Scene/SceneManager.h"

MainMenuScene::MainMenuScene()
    : _name("MainMenuScene"),
      _promptText(ResourceManager::getInstance().getFont("Roboto"), "Press any key to continue", 48) {
    sf::FloatRect bounds = _promptText.getLocalBounds();
    _promptText.setOrigin({bounds.position.x + bounds.size.x / 2.0f,
                           bounds.position.y + bounds.size.y / 2.0f});
    _promptText.setPosition({960.0f, 540.0f});
    _promptText.setFillColor(sf::Color::Black);
}

void MainMenuScene::init() {
}

void MainMenuScene::onEnter() {
    _isActive = true;

    if (!_showPrompt) {
        _setupButtons();
    }
}

void MainMenuScene::onExit() {
    _isActive = false;
}

void MainMenuScene::cleanup() {
}

void MainMenuScene::handleInput(const sf::Event& event) {
    if (_showPrompt) {
        if (event.is<sf::Event::KeyPressed>() ||
            event.is<sf::Event::MouseButtonPressed>() ||
            event.is<sf::Event::JoystickButtonPressed>()) {
            _showPrompt = false;
            _setupButtons();
        }
        return;
    }

    _buttonMenu.processEvent(event);
}

void MainMenuScene::update(float deltaTime) {
    (void)deltaTime;
}

void MainMenuScene::render(sf::RenderTarget& target) {
    if (_showPrompt) {
        target.draw(_promptText);
        return;
    }

    _buttonMenu.render(target);
}

void MainMenuScene::_setupButtons() {
    _buttonMenu.clear();
    _buttonMenu.setLayoutProperties(
        {820.0f, 398.0f},
        {280.0f, 60.0f},
        75.0f,
        false,
        sf::Color(100, 149, 237),
        28
    );

    _buttonMenu.addButtonAuto("New Game", std::make_unique<FunctionalCommand>(
        "New Game", [this]() {
            if (auto mgr = getSceneManager()) {
                mgr->pushSceneByName("IN_GAME");
            }
        }
    ));

    _buttonMenu.addButtonAuto("Load Game", std::make_unique<FunctionalCommand>(
        "Load Game", [this]() {
            if (auto mgr = getSceneManager()) {
                mgr->pushSceneByName("GAME_DATA");
            }
        }
    ));

    _buttonMenu.addButtonAuto("Settings", std::make_unique<FunctionalCommand>(
        "Settings", [this]() {
            if (auto mgr = getSceneManager()) {
                mgr->pushSceneByName("SETTINGS");
            }
        }
    ));

    _buttonMenu.addButtonAuto("Exit Game", std::make_unique<FunctionalCommand>(
        "Exit Game", [this]() {
            if (auto mgr = getSceneManager()) {
                if (auto wnd = mgr->getRenderWindow()) {
                    wnd->close();
                }
            }
        }
    ));
}
