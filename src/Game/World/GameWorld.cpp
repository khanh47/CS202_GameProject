#include "Game/World/GameWorld.h"
#include "Game/Behaviours/Controllable.h"

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

}

void GameWorld::render(sf::RenderTarget &target) {
    for(auto object: _objects)
        object->render(target);
}

void GameWorld::test() {
    auto player1 = _objectFactory.createPlayer();
    player1->spawn(_physicsWorld, {500, 500}, {64, 123});

    auto player2 = _objectFactory.createPlayer();
    player2->spawn(_physicsWorld, {625, 555}, {36, 36});

    auto& tilesTexture = ResourceManager::getInstance().getTexture("brick");
    auto tileBlock = _objectFactory.createBlock("Block", &tilesTexture);
    tileBlock->spawn(_physicsWorld, {666, 666}, {32, 32});

    _objects.push_back(player1);
    _objects.push_back(player2);
    _objects.push_back(tileBlock);
}