#include "Scene/ConcreteScene/LoadGameScene.h"

#include "Commands/FunctionalCommand.h"
#include "Game/Snapshot/SaveLoadGame.h"
#include "ResourceManager.h"
#include "Scene/ConcreteScene/InGameScene.h"
#include "Scene/SceneManager.h"

LoadGameScene::LoadGameScene()
    : Scene("LoadGameScene"),
      _titleText(
          ResourceManager::getInstance().getFont("SuperMario"),
          "LOAD GAME",
          64
      ),
      _statusText(
          ResourceManager::getInstance().getFont("moon_get"),
          "",
          24
      ) {
    _titleText.setOutlineThickness(5.0f);
    _titleText.setOutlineColor(sf::Color::Black);
    _titleText.setFillColor(sf::Color::White);
    const sf::FloatRect titleBounds = _titleText.getLocalBounds();
    _titleText.setOrigin({
        titleBounds.position.x + titleBounds.size.x * 0.5f,
        titleBounds.position.y + titleBounds.size.y * 0.5f
    });
    _titleText.setPosition({960.0f, 180.0f});
}

void LoadGameScene::onEnter() {
    Scene::onEnter();
    setupButtons();
    if (!SaveLoadGame::getInstance().hasAnySave()) {
        setStatus(
            "No saved default-level games found.",
            sf::Color(255, 220, 120)
        );
    }
}

void LoadGameScene::handleInput(const sf::Event& event) {
    if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
        if (keyEvent->code == sf::Keyboard::Key::Escape) {
            if (auto* manager = getSceneManager()) {
                manager->requestPopScene();
                return;
            }
        }
    }
    _buttonMenu.processEvent(event);
}

void LoadGameScene::render(sf::RenderTarget& target) {
    Scene::render(target);
    target.draw(_titleText);
    target.draw(_statusText);
    _buttonMenu.render(target);
}

void LoadGameScene::setStatus(const std::string& status, sf::Color color) {
    _statusText.setString(status);
    _statusText.setFillColor(color);
    const sf::FloatRect bounds = _statusText.getLocalBounds();
    _statusText.setOrigin({
        bounds.position.x + bounds.size.x * 0.5f,
        bounds.position.y + bounds.size.y * 0.5f
    });
    _statusText.setPosition({960.0f, 245.0f});
}

void LoadGameScene::loadFromSlot(int index) {
    SaveLoadGame::SlotInfo slot;
    nlohmann::json state;
    if (!SaveLoadGame::getInstance().loadSlot(index, slot, state)) {
        setStatus("This save slot is empty or invalid.", sf::Color(255, 120, 120));
        return;
    }

    const std::string levelPath = state.value("levelPath", "");
    const std::string gameMode = state.value("gameMode", "");
    if (levelPath.empty() || gameMode == "minigame") {
        setStatus("This slot is not a default-level game.", sf::Color(255, 120, 120));
        return;
    }

    if (auto* manager = getSceneManager()) {
        manager->pushScene(std::make_unique<InGameScene>(
            levelPath,
            std::optional<nlohmann::json>{std::move(state)}
        ));
    }
}

void LoadGameScene::setupButtons() {
    _buttonMenu.clear();
    _buttonMenu.setLayoutProperties(
        {650.0f, 300.0f},
        {620.0f, 78.0f},
        88.0f,
        false,
        sf::Color(100, 149, 237),
        22
    );

    const std::vector<SaveLoadGame::SlotInfo> slots =
        SaveLoadGame::getInstance().getSlots();
    for (int index = 0; index < SaveLoadGame::SlotCount; ++index) {
        const SaveLoadGame::SlotInfo& slot = slots[static_cast<std::size_t>(index)];
        const std::string label = slot.exists
            ? "Slot " + std::to_string(index + 1) + ": " + slot.name
                + "\nSaved: " + slot.savedDate
            : "Slot " + std::to_string(index + 1) + ": Empty";

        _buttonMenu.addButtonAuto(
            label,
            22,
            std::make_unique<FunctionalCommand>(
                "Load Slot " + std::to_string(index + 1),
                [this, index]() { loadFromSlot(index); }
            )
        );
    }

    _buttonMenu.addButtonAuto(
        "Back",
        std::make_unique<FunctionalCommand>(
            "Back",
            [this]() {
                if (auto* manager = getSceneManager()) {
                    manager->requestPopScene();
                }
            }
        )
    );
}
