#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include "SceneManager.h"
#include "SceneFactory.h"

class App {
private:
    std::unique_ptr<SceneManager> manager;
    std::unique_ptr<SceneFactory> factory;
    sf::RenderWindow window;
    sf::Clock dtClock;
    void render();
    void update(float deltaTime);
    void processEvents();

public:
    App();
    void run();
};