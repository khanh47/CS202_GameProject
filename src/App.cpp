#include "App.h"

namespace {
    constexpr double fixedDt = 1.0 / 60.0;
}

App::App() : window(sf::VideoMode({1920, 1080}), "SUPER MARIO") {
    factory = std::make_unique<SceneFactory>();
    manager = std::make_unique<SceneManager>(factory.get());
    manager->setRenderWindow(&window);
    window.setFramerateLimit(60);
    manager->pushSceneByName("MAIN_MENU");
}

void App::run() {
    dtClock.restart();
    while (window.isOpen()) {
        float deltaTime = dtClock.restart().asSeconds();
        if(manager->getSceneName() == "IN_GAME")
            accumulatedTime += std::min(deltaTime, 0.25f); // min to prevent too many physics updates at once => feel glitchy, not smooth LOL

        processEvents();
        updateSimulation(fixedDt);
        updateVisuals(deltaTime);
        render();
    }
}

void App::processEvents() {
    while (const std::optional<sf::Event> event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
        }
        manager->processEvents(*event);
    }
}

void App::updateSimulation(const float &fixedDt){
    while(accumulatedTime >= fixedDt){
        if(manager->getSceneName() == "IN_GAME"){
            manager->updateSimulation(fixedDt);
        }
        accumulatedTime -= fixedDt;
    }
}

// Visual update
void App::updateVisuals(float deltaTime) {
    manager->updateVisuals(deltaTime);
}

void App::render() {
    window.clear(sf::Color(240, 240, 240));
    manager->render(window);
    window.display();
}