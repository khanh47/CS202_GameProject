#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include "Scene/SceneManager.h"
#include "Scene/SceneFactory.h"

class App {
private:
    std::unique_ptr<SceneManager> manager;
    std::unique_ptr<SceneFactory> factory;
    sf::RenderWindow window;
    sf::Clock dtClock;
    double accumulatedTime = 0.0f;
    void render();
    void updateSimulation(const float &fixedDt);
    void updateVisuals(float deltaTime); // Visuals
    void processEvents();

public:
    App();
    void run();
};