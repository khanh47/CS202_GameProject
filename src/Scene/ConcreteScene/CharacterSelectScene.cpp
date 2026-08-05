#include "Scene/ConcreteScene/CharacterSelectScene.h"
#include "Commands/FunctionalCommand.h"
#include "Game/GameSettings.h"
#include "ResourceManager.h"
#include "Scene/SceneManager.h"

CharacterSelectScene::CharacterSelectScene()
    : _name("CharacterSelectScene"),
      _titleText(ResourceManager::getInstance().getFont("Roboto"), "PICK A CHARACTER", 64) {
    sf::FloatRect bounds = _titleText.getLocalBounds();
    _titleText.setOrigin({bounds.position.x + bounds.size.x / 2.0f,
                          bounds.position.y + bounds.size.y / 2.0f});
    _titleText.setPosition({960.0f, 220.0f});
    _titleText.setFillColor(sf::Color::Black);
}

void CharacterSelectScene::init() {
}

void CharacterSelectScene::onEnter() {
    _isActive = true;
    _setupButtons();
}

void CharacterSelectScene::onExit() {
    _isActive = false;
}

void CharacterSelectScene::cleanup() {
}

void CharacterSelectScene::handleInput(const sf::Event& event) {
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

void CharacterSelectScene::updateVisuals(float deltaTime) {
    (void)deltaTime;
}

void CharacterSelectScene::render(sf::RenderTarget& target) {
    target.draw(_titleText);
    _buttonMenu.render(target);
}

void CharacterSelectScene::_setupButtons() {
    _buttonMenu.clear();
    _buttonMenu.setLayoutProperties(
        {820.0f, 360.0f},
        {280.0f, 60.0f},
        75.0f,
        false,
        sf::Color(100, 149, 237),
        28
    );

    _buttonMenu.addButtonAuto("Mario", std::make_unique<FunctionalCommand>(
        "Mario", [this]() {
            GameSettings::getInstance().player1Character = "mario";
            if (auto mgr = getSceneManager()) {
                mgr->pushSceneByName("LEVEL_SELECT");
            }
        }
    ));

    _buttonMenu.addButtonAuto("Luigi", std::make_unique<FunctionalCommand>(
        "Luigi", [this]() {
            GameSettings::getInstance().player1Character = "luigi";
            if (auto mgr = getSceneManager()) {
                mgr->pushSceneByName("LEVEL_SELECT");
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
