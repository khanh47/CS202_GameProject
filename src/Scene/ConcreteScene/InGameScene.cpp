#include <memory>

#include "Scene/ConcreteScene/InGameScene.h"
#include "ResourceManager.h"
#include "Scene/SceneManager.h"
#include "Game/Objects/Player.h"

InGameScene::InGameScene(const std::string& name)
    : _name(name) {
        // spawn player 1
        auto player1 = std::make_shared<Player>();
        _objects.push_back(player1);
        _objects.back()->spawn(_gameWorld.get(), {500, 500}, {64, 123}); 

        // spawn player 2
        auto player2 = std::make_shared<Player>();
        _objects.push_back(player2);
        _objects.back()->spawn(_gameWorld.get(), {666, 666}, {36, 36}); 

        // spawn a "player" with the texture of a brick =))
        auto player3 = std::make_shared<Player>(ResourceManager::getInstance().getTexture("brick"));
        _objects.push_back(player3);
        _objects.back()->spawn(_gameWorld.get(), {666, 666}, {36, 36}); 
}

void InGameScene::init() {
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

    for(std::shared_ptr<GameObject> object: _objects) object->render(target);
}