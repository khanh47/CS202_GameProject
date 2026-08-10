#include "Scene/ConcreteScene/CharacterSelectScene.h"
#include "Commands/FunctionalCommand.h"
#include "Game/GameSettings.h"
#include "ResourceManager.h"
#include "Scene/SceneManager.h"

CharacterSelectScene::CharacterSelectScene()
    : Scene("CharacterSelectScene"),
      _titleText(ResourceManager::getInstance().getFont("SuperMario"), "PICK A CHARACTER", 64) {
    sf::FloatRect bounds = _titleText.getLocalBounds();
    _titleText.setOutlineThickness(5.0f);
    _titleText.setOutlineColor(sf::Color::Black);
    _titleText.setFillColor(sf::Color::White);
    _titleText.setOrigin({bounds.position.x + bounds.size.x / 2.0f,
                          bounds.position.y + bounds.size.y / 2.0f});
    _titleText.setPosition({960.0f, 220.0f});
    _titleText.setFillColor(sf::Color::White);
}

void CharacterSelectScene::onEnter() {
    Scene::onEnter();
    _setupButtons();
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

void CharacterSelectScene::render(sf::RenderTarget& target) {
    Scene::render(target);
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
