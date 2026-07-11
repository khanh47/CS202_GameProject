#include "App.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace {
    constexpr double fixedDt = 1.0 / 60.0;
}

App::App() : window(sf::VideoMode({1920, 1080}), "SUPER MARIO") {
#ifdef _WIN32
    ShowWindow((HWND)window.getNativeHandle(), SW_MAXIMIZE); //to maximize window
#endif
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
        // updateVisuals(std::min(deltaTime, 0.25f));
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
            manager->updateVisuals(fixedDt);
        }
        accumulatedTime -= fixedDt;
    }
}

// Visual update
void App::updateVisuals(float deltaTime) {
    manager->updateVisuals(deltaTime); //not controlled => 19283012 updates a second -> crash
}

void App::render() {
    window.clear(sf::Color(240, 240, 240));
    manager->render(window);
    window.display();
}