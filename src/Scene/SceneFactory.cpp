#include "Scene/SceneFactory.h"
#include "Scene/ConcreteScene/InGameScene.h"
#include "Scene/ConcreteScene/MainMenuScene.h"
#include "Scene/ConcreteScene/SettingsScene.h"
#include <iostream>

SceneFactory::SceneFactory() {
    registerScene("MAIN_MENU", []() { return std::make_unique<MainMenuScene>(); });
    //registerScene("GAME_DATA", []() { return std::make_unique<MenuScene>("Load Game"); });
    registerScene("SETTINGS", []() { return std::make_unique<SettingsScene>(); });
    registerScene("IN_GAME", []() { return std::make_unique<InGameScene>("NONE"); });
}

void SceneFactory::registerScene(const std::string& stateName, std::function<std::unique_ptr<Scene>()> factory) {
    _factories[stateName] = std::move(factory);
}

std::unique_ptr<Scene> SceneFactory::createScene(const std::string& stateName) {
    auto it = _factories.find(stateName);
    if (it != _factories.end()) {
        return it->second();
    }
    std::cerr << "Error: Unknown scene for state: " << stateName << std::endl;
    return nullptr;
}
