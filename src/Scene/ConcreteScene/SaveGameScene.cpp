#include "Scene/ConcreteScene/SaveGameScene.h"

#include "Button/TextInput.h"
#include "Commands/FunctionalCommand.h"
#include "Game/Snapshot/SaveLoadGame.h"
#include "ResourceManager.h"
#include "Scene/SceneManager.h"

SaveGameScene::SaveGameScene(bool exitAfterSave)
    : Scene("SaveGameScene"),
      _exitAfterSave(exitAfterSave),
      _titleText(
          ResourceManager::getInstance().getFont("SuperMario"),
          "SAVE GAME",
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
    _titleText.setPosition({960.0f, 150.0f});
    _statusText.setFillColor(sf::Color::White);
}

void SaveGameScene::onEnter() {
    Scene::onEnter();
    setupControls();
    if (!SaveLoadGame::getInstance().hasCurrentSession()) {
        setStatus("No active default-level game to save.", sf::Color(255, 220, 120));
    }
}

void SaveGameScene::handleInput(const sf::Event& event) {
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

void SaveGameScene::render(sf::RenderTarget& target) {
    Scene::render(target);
    target.draw(_titleText);
    target.draw(_statusText);
    _buttonMenu.render(target);
}

void SaveGameScene::setStatus(const std::string& status, sf::Color color) {
    _statusText.setString(status);
    _statusText.setFillColor(color);
    const sf::FloatRect bounds = _statusText.getLocalBounds();
    _statusText.setOrigin({
        bounds.position.x + bounds.size.x * 0.5f,
        bounds.position.y + bounds.size.y * 0.5f
    });
    _statusText.setPosition({960.0f, 225.0f});
}

void SaveGameScene::saveToSlot(int index) {
    const nlohmann::json* currentSession =
        SaveLoadGame::getInstance().getCurrentSession();
    if (!currentSession) {
        setStatus("No active default-level game to save.", sf::Color(255, 220, 120));
        return;
    }

    std::string slotName = _slotInputs[static_cast<std::size_t>(index)]->getValue();
    if (slotName.empty()) {
        slotName = "Slot " + std::to_string(index + 1);
    }

    if (!SaveLoadGame::getInstance().saveSlot(index, slotName, *currentSession)) {
        setStatus("Could not save this slot.", sf::Color(255, 120, 120));
        return;
    }

    SaveLoadGame::getInstance().markSessionSaved();
    setStatus(
        "Saved " + slotName + ".",
        sf::Color(170, 255, 170)
    );

    if (_exitAfterSave) {
        if (auto* manager = getSceneManager()) {
            if (auto* window = manager->getRenderWindow()) {
                window->close();
            }
        }
    }
}

void SaveGameScene::setupControls() {
    _buttonMenu.clear();
    _slotInputs.clear();
    // Save slots are intentionally keyboard-only. ButtonMenu forwards the
    // focused control's text and editing keys, while mouse events are locked
    // out for this screen.
    _buttonMenu.setMouseOnly(false);
    _buttonMenu.setArrowKeysOnly(true);
    _buttonMenu.setLayoutProperties(
        {700.0f, 275.0f},
        {520.0f, 58.0f},
        72.0f,
        false,
        sf::Color(100, 149, 237),
        24
    );

    const std::vector<SaveLoadGame::SlotInfo> slots =
        SaveLoadGame::getInstance().getSlots();
    for (int index = 0; index < SaveLoadGame::SlotCount; ++index) {
        std::string initialName = "";
        if (index < static_cast<int>(slots.size()) && slots[index].exists) {
            initialName = slots[index].name;
        }

        auto input = std::make_shared<UI::TextInput>(
            sf::Vector2f{},
            sf::Vector2f{520.0f, 58.0f},
            sf::Color(53, 91, 130),
            "Slot " + std::to_string(index + 1) + " name",
            24,
            initialName
        );
        input->setMaxLength(20);
        _slotInputs.push_back(input);
        _buttonMenu.addButton(input);

        _buttonMenu.addButtonAuto(
            "Save Slot " + std::to_string(index + 1),
            std::make_unique<FunctionalCommand>(
                "Save Slot " + std::to_string(index + 1),
                [this, index]() { saveToSlot(index); }
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

    // TextInput controls are added directly, so apply the common layout after
    // every control has been inserted.
    _buttonMenu.setLayoutProperties(
        {700.0f, 275.0f},
        {520.0f, 58.0f},
        72.0f,
        false,
        sf::Color(100, 149, 237),
        24
    );
}
