#include "App.h"

App::App() : window(sf::VideoMode({1920, 1080}), "SUPER MARIO") {
    factory = std::make_unique<SceneFactory>();
    manager = std::make_unique<SceneManager>(factory.get());
    manager->setRenderWindow(&window);
    window.setFramerateLimit(60);
    manager->pushSceneByName("MAIN_MENU");
}

void App::run() {
    while (window.isOpen()) {
        float deltaTime = dtClock.restart().asSeconds();
        processEvents();
        update(deltaTime);
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

void App::update(float deltaTime) {
    manager->update(deltaTime);
}

void App::render() {
    window.clear(sf::Color(240, 240, 240));
    manager->render(window);
    window.display();
}