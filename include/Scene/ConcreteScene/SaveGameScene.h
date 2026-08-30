#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "Button/ButtonMenu.h"
#include "Scene/Scene.h"

namespace UI {
class TextInput;
}

class SaveGameScene : public Scene {
public:
    explicit SaveGameScene(
        bool exitAfterSave = false,
        std::function<void()> onSuccessfulSave = {}
    );
    ~SaveGameScene() override = default;

    void onEnter() override;
    void handleInput(const sf::Event& event) override;
    void updateVisuals(float deltaTime) override;
    void render(sf::RenderTarget& target) override;

private:
    void setupControls();
    void saveToSlot(int index);
    void setStatus(const std::string& status, sf::Color color = sf::Color::White);

    bool _exitAfterSave = false;
    std::function<void()> _onSuccessfulSave;
    UI::ButtonMenu _buttonMenu;
    std::vector<std::shared_ptr<UI::TextInput>> _slotInputs;
    sf::Text _titleText;
    sf::Text _statusText;
};
