#pragma once

#include "Scene/Scene.h"
#include "Button/ButtonMenu.h"
#include "Button/BarSlider.h"
#include "Game/UserInput/Action.h"

#include <memory>
#include <optional>

class SettingsScene : public Scene {
public:
    SettingsScene();
    ~SettingsScene() override = default;

    void init() override;
    void onEnter() override;
    void onExit() override;

    void handleInput(const sf::Event& event) override;
    void updateVisuals(float deltaTime) override;
    void render(sf::RenderTarget& target) override;

    void startRebinding(ActionType action, std::shared_ptr<UI::Button> button);

private:
    enum class SettingsTab { Music, Keybind, DevMode };
    enum class FocusTarget { TopMenu, SubMenu };

    void buildTopMenu();
    void buildMusicPanel();
    void buildKeybindPanel();
    void buildDevModePanel();
    void setActiveTab(SettingsTab tab);
    void updateKeybindButtonTexts();
    void updateVolumeLabel();
    void navigateMenu(UI::ButtonMenu& menu, int delta);

    UI::ButtonMenu _topMenu;
    UI::ButtonMenu _subMenu;

    SettingsTab _activeTab = SettingsTab::Music;
    FocusTarget _focus = FocusTarget::SubMenu;

    std::shared_ptr<UI::Button> _btnMoveLeft;
    std::shared_ptr<UI::Button> _btnMoveRight;
    std::shared_ptr<UI::Button> _btnJump;
    std::shared_ptr<UI::Button> _btnAttack;
    std::shared_ptr<UI::Button> _btnButton;
    std::shared_ptr<UI::BarSlider> _btnMusicVolume;
    std::shared_ptr<UI::BarSlider> _btnSoundVolume;

    std::optional<ActionType> _rebindingAction;
    std::shared_ptr<UI::Button> _activeRebindingButton;
};
