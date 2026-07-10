#include "Game/World/GameWorld.h"
#include "Game/Behaviours/Controllable.h"
#include "ResourceManager.h"

GameWorld::GameWorld() {

}

void GameWorld::handleInput(const sf::Event& event) {
    for (auto& obj : _objects) {
        auto* controllable = dynamic_cast<Controllable*>(obj.get());
        if (controllable) {
            controllable->onInput(event);
        }
    }
}

void GameWorld::updateSimulation(const float &fixedDt) {
    _physicsWorld.updateSimulation(fixedDt);
}

void GameWorld::updateVisuals(float deltaTime) {
    for(auto object: _objects)
        object->updateVisuals(deltaTime);
}

void GameWorld::render(sf::RenderTarget &target) {
    for(auto object: _objects)
        object->render(target);
}

void GameWorld::test() {
    auto& marioTexture = ResourceManager::getInstance().getTexture("mario_spritesheet");
    auto player1 = _objectFactory.createPlayer("Player", &marioTexture);
    player1->spawn(_physicsWorld, {500, 500}, {128, 128});

    auto& luigiTexture = ResourceManager::getInstance().getTexture("luigi_spritesheet");
    auto player2 = _objectFactory.createPlayer("Player", &luigiTexture);
    player2->spawn(_physicsWorld, {625, 555}, {128, 128});

    auto& brickTexture = ResourceManager::getInstance().getTexture("brick");
    auto brickBlock = _objectFactory.createBlock("Block", &brickTexture);
    brickBlock->spawn(_physicsWorld, {0, 888}, {9999, 32});

    _objects.push_back(player1);
    _objects.push_back(player2);
    _objects.push_back(brickBlock);
}