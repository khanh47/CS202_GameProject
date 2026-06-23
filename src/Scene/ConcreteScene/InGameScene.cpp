#include <memory>

#include "Scene/ConcreteScene/InGameScene.h"
#include "ResourceManager.h"
#include "Scene/SceneManager.h"

InGameScene::InGameScene(const std::string& name)
    : _name(name) {
}

void InGameScene::init() {
    auto player1 = _objectFactory.createPlayer();
    player1->spawn(_gameWorld.get(), {500, 500}, {64, 123});

    auto player2 = _objectFactory.createPlayer();
    player2->spawn(_gameWorld.get(), {625, 555}, {36, 36});

    auto& brickTexture = ResourceManager::getInstance().getTexture("brick");
    auto brickBlock = _objectFactory.createBlock("Block", &brickTexture);
    brickBlock->spawn(_gameWorld.get(), {666, 666}, {32, 32});
}

void InGameScene::onEnter() {
    _isActive = true;
}

void InGameScene::onExit() {
    _isActive = false;
}

void InGameScene::cleanup() {
}

void InGameScene::handleInput(const sf::Event& event) {
    if (auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
        if (keyEvent->code == sf::Keyboard::Key::Escape) {
            if (auto mgr = getSceneManager()) {
                mgr->popScene();
            }
        }
    }
}

void InGameScene::updateSimulation(const float &fixedDt) {
    // update the game, physics and stuff here
    // std::cout << "FUNNY EH?" << std::endl;
    _gameWorld->step(fixedDt);
}

void InGameScene::updateVisuals(float deltaTime) {
    (void)deltaTime;
}

void InGameScene::render(sf::RenderTarget& target) {
    (void)target;
    const sf::Font& font = ResourceManager::getInstance().getFont("Roboto");
    sf::Text text(font, "InGame scene", 24);
    text.setFillColor(sf::Color::Black);
    target.draw(text);

    _objectFactory.render(target);
}
