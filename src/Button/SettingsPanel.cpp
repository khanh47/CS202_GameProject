#include "Button/SettingsPanel.h"

#include <algorithm>
#include <array>
#include <string>
#include <utility>

#include "Audio/MusicManager.h"
#include "Audio/SoundManager.h"
#include "Commands/FunctionalCommand.h"
#include "Commands/ToggleDebugCommands.h"
#include "Game/GameSettings.h"
#include "ResourceManager.h"

namespace UI {
namespace {
struct KeybindDescriptor {
    const char* label;
    ActionType action;
};

constexpr std::array<KeybindDescriptor, 6> keybinds{{
    {"Move Left", ActionType::MoveLeft},
    {"Move Right", ActionType::MoveRight},
    {"Jump", ActionType::MoveUp},
    {"Pipe Down", ActionType::MoveDown},
    {"Shoot", ActionType::Attack},
    {"Button", ActionType::Interact}
}};
}

SettingsPanel::SettingsPanel(BackCallback onBack)
    : _onBack(std::move(onBack)) {
    buildTopMenu();
    setActiveTab(SettingsTab::Music);
}

void SettingsPanel::setOnBack(BackCallback onBack) {
    _onBack = std::move(onBack);
}

void SettingsPanel::refresh() {
    _rebindingAction.reset();
    _activeRebindingButton.reset();
    setActiveTab(_activeTab);
}

void SettingsPanel::buildTopMenu() {
    _topMenu.clear();
    _topMenu.setLayoutProperties(
        {515.f, 180.f}, {200.f, 60.f}, 230.f, true,
        sf::Color(70, 130, 180), 26
    );
    _topMenu.addButtonAuto("Music", std::make_unique<FunctionalCommand>(
        "TabMusic", [this]() { setActiveTab(SettingsTab::Music); }
    ));
    _topMenu.addButtonAuto("Keybind", std::make_unique<FunctionalCommand>(
        "TabKeybind", [this]() { setActiveTab(SettingsTab::Keybind); }
    ));
    _topMenu.addButtonAuto("Dev Mode", std::make_unique<FunctionalCommand>(
        "TabDevMode", [this]() { setActiveTab(SettingsTab::DevMode); }
    ));
    _topMenu.addButtonAuto(
        "Back", 26,
        std::make_unique<FunctionalCommand>("Back", [this]() { goBack(); }),
        sf::Color(180, 80, 80)
    );
}

void SettingsPanel::buildMusicPanel() {
    const sf::Vector2f start{780.f, 315.f};
    const sf::Vector2f size{360.f, 60.f};
    constexpr float spacing = 82.f;
    const sf::Color color(70, 130, 180);
    _subMenu.setLayoutProperties(start, size, spacing, false, color, 24);

    auto& settings = GameSettings::getInstance();
    auto musicToggle = std::make_shared<ToggleButton>(
        start, size, color, "Music Enabled", 24,
        settings.musicEnabled, 20.f
    );
    musicToggle->setToggleCallback([](bool enabled) {
        auto& current = GameSettings::getInstance();
        current.musicEnabled = enabled;
        Audio::MusicManager::getInstance().setEnabled(enabled);
        current.save();
    });
    _subMenu.addButton(musicToggle);

    _musicVolume = std::make_shared<BarSlider>(
        sf::Vector2f(start.x, start.y + spacing), size, color,
        "Music Volume", 24, settings.musicVolume, 0.f, 100.f, false, 20.f
    );
    _musicVolume->setValueCallback([this](float value) {
        auto& current = GameSettings::getInstance();
        current.musicVolume = std::clamp(value, 0.f, 100.f);
        Audio::MusicManager::getInstance().setVolume(current.musicVolume);
        current.save();
        updateVolumeLabels();
    });
    _subMenu.addButton(_musicVolume);

    auto soundToggle = std::make_shared<ToggleButton>(
        sf::Vector2f(start.x, start.y + spacing * 2.f), size, color,
        "Sound Enabled", 24, settings.soundEnabled, 20.f
    );
    soundToggle->setToggleCallback([](bool enabled) {
        auto& current = GameSettings::getInstance();
        current.soundEnabled = enabled;
        Audio::SoundManager::getInstance().setEnabled(enabled);
        current.save();
    });
    _subMenu.addButton(soundToggle);

    _soundVolume = std::make_shared<BarSlider>(
        sf::Vector2f(start.x, start.y + spacing * 3.f), size, color,
        "Sound Volume", 24, settings.soundVolume, 0.f, 100.f, false, 20.f
    );
    _soundVolume->setValueCallback([this](float value) {
        auto& current = GameSettings::getInstance();
        current.soundVolume = std::clamp(value, 0.f, 100.f);
        Audio::SoundManager::getInstance().setGlobalVolume(current.soundVolume);
        current.save();
        updateVolumeLabels();
    });
    _subMenu.addButton(_soundVolume);
    updateVolumeLabels();
}

void SettingsPanel::buildKeybindPanel() {
    const sf::Vector2f start{780.f, 285.f};
    const sf::Vector2f size{360.f, 54.f};
    constexpr float spacing = 65.f;
    const sf::Color color(70, 130, 180);
    _subMenu.setLayoutProperties(start, size, spacing, false, color, 22);

    _playerToggle = std::make_shared<ToggleButton>(
        start, size, color, "Player 2 Keys", 22, _editingPlayer2, 18.f
    );
    _playerToggle->setToggleCallback([this](bool player2) {
        _editingPlayer2 = player2;
        setActiveTab(SettingsTab::Keybind);
    });
    _subMenu.addButton(_playerToggle);

    _keybindButtons.clear();
    _keybindButtons.reserve(keybinds.size());
    for (std::size_t index = 0; index < keybinds.size(); ++index) {
        const KeybindDescriptor descriptor = keybinds[index];
        auto button = std::make_shared<Button>(
            sf::Vector2f(start.x, start.y + spacing * static_cast<float>(index + 1)),
            size, color, "", 22, 18.f
        );
        const std::weak_ptr<Button> weakButton = button;
        button->setCommand(std::make_unique<FunctionalCommand>(
            std::string("Rebind") + descriptor.label,
            [this, descriptor, weakButton]() {
                startRebinding(descriptor.action, weakButton.lock());
            }
        ));
        _keybindButtons.push_back(button);
        _subMenu.addButton(button);
    }
    updateKeybindButtonTexts();
}

void SettingsPanel::buildDevModePanel() {
    _subMenu.setLayoutProperties(
        {780.f, 315.f}, {360.f, 60.f}, 82.f, false,
        sf::Color(100, 149, 237), 24
    );
    const auto& settings = GameSettings::getInstance();
    _subMenu.addToggleButtonAuto(
        "Grid", settings.debugDrawGrid, std::make_unique<ToggleGridCommand>()
    );
    _subMenu.addToggleButtonAuto(
        "Coordinates", settings.debugDrawCoordinates,
        std::make_unique<ToggleCoordinatesCommand>()
    );
    _subMenu.addToggleButtonAuto(
        "Hitbox", settings.debugDrawHitbox, std::make_unique<ToggleHitboxCommand>()
    );
    _subMenu.addToggleButtonAuto(
        "Camera Move", settings.freeCameraMove,
        std::make_unique<ToggleFreeCameraCommand>()
    );
}

void SettingsPanel::setActiveTab(SettingsTab tab) {
    _activeTab = tab;
    _subMenu.clear();
    _keybindButtons.clear();
    _playerToggle.reset();
    _musicVolume.reset();
    _soundVolume.reset();
    switch (_activeTab) {
        case SettingsTab::Music: buildMusicPanel(); break;
        case SettingsTab::Keybind: buildKeybindPanel(); break;
        case SettingsTab::DevMode: buildDevModePanel(); break;
    }
    _subMenu.setFocusedIndex(0);
    _focus = FocusTarget::SubMenu;
}

void SettingsPanel::startRebinding(
    ActionType action,
    const std::shared_ptr<Button>& button
) {
    _rebindingAction = action;
    _activeRebindingButton = button;
    if (button) button->setText("< Press Key >");
}

void SettingsPanel::updateKeybindButtonTexts() {
    if (_keybindButtons.size() != keybinds.size()) return;
    const auto& settings = GameSettings::getInstance();
    for (std::size_t index = 0; index < keybinds.size(); ++index) {
        const sf::Keyboard::Key key = _editingPlayer2
            ? settings.getKeyForAction2(keybinds[index].action)
            : settings.getKeyForAction(keybinds[index].action);
        _keybindButtons[index]->setText(
            std::string(keybinds[index].label) + ": "
            + GameSettings::keyToString(key)
        );
    }
}

void SettingsPanel::updateVolumeLabels() {
    const auto& settings = GameSettings::getInstance();
    if (_musicVolume) {
        _musicVolume->setText(
            "Music Volume: " + std::to_string(static_cast<int>(settings.musicVolume)) + "%"
        );
    }
    if (_soundVolume) {
        _soundVolume->setText(
            "Sound Volume: " + std::to_string(static_cast<int>(settings.soundVolume)) + "%"
        );
    }
}

void SettingsPanel::navigateMenu(ButtonMenu& menu, int delta) {
    const int count = static_cast<int>(menu.size());
    if (count > 0) {
        menu.setFocusedIndex((menu.getFocusedIndex() + delta + count) % count);
    }
}

void SettingsPanel::goBack() {
    _rebindingAction.reset();
    _activeRebindingButton.reset();
    if (_onBack) _onBack();
}

void SettingsPanel::handleInput(const sf::Event& event) {
    if (_rebindingAction) {
        if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
            if (key->code == sf::Keyboard::Key::Escape) {
                updateKeybindButtonTexts();
                _rebindingAction.reset();
                _activeRebindingButton.reset();
            } else if (key->code != sf::Keyboard::Key::Unknown) {
                auto& settings = GameSettings::getInstance();
                if (_editingPlayer2) {
                    settings.setKeyForAction2(*_rebindingAction, key->code);
                } else {
                    settings.setKeyForAction(*_rebindingAction, key->code);
                }
                settings.save();
                updateKeybindButtonTexts();
                _rebindingAction.reset();
                _activeRebindingButton.reset();
            }
        }
        return;
    }

    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (_focus == FocusTarget::SubMenu && _activeTab == SettingsTab::Music) {
            auto focused = _subMenu.getButton(
                static_cast<std::size_t>(_subMenu.getFocusedIndex())
            );
            if (auto slider = std::dynamic_pointer_cast<BarSlider>(focused);
                slider && slider->isSelected()) {
                if (key->code == sf::Keyboard::Key::Left
                    || key->code == sf::Keyboard::Key::A) {
                    slider->adjust(-5.f);
                    return;
                }
                if (key->code == sf::Keyboard::Key::Right
                    || key->code == sf::Keyboard::Key::D) {
                    slider->adjust(5.f);
                    return;
                }
            }
        }

        switch (key->code) {
            case sf::Keyboard::Key::Escape: goBack(); return;
            case sf::Keyboard::Key::Left:
            case sf::Keyboard::Key::A:
                _focus = FocusTarget::TopMenu;
                navigateMenu(_topMenu, -1);
                return;
            case sf::Keyboard::Key::Right:
            case sf::Keyboard::Key::D:
                _focus = FocusTarget::TopMenu;
                navigateMenu(_topMenu, 1);
                return;
            case sf::Keyboard::Key::Up:
            case sf::Keyboard::Key::W:
                _focus = FocusTarget::SubMenu;
                navigateMenu(_subMenu, -1);
                return;
            case sf::Keyboard::Key::Down:
            case sf::Keyboard::Key::S:
                _focus = FocusTarget::SubMenu;
                navigateMenu(_subMenu, 1);
                return;
            case sf::Keyboard::Key::Enter:
            case sf::Keyboard::Key::Space: {
                ButtonMenu& menu = _focus == FocusTarget::TopMenu
                    ? _topMenu : _subMenu;
                if (auto button = menu.getButton(
                        static_cast<std::size_t>(menu.getFocusedIndex()))) {
                    button->execute();
                }
                return;
            }
            default: return;
        }
    }

    _topMenu.processEvent(event);
    _subMenu.processEvent(event);
    if (event.is<sf::Event::MouseMoved>()) {
        for (std::size_t index = 0; index < _topMenu.size(); ++index) {
            if (auto button = _topMenu.getButton(index); button && button->isFocused()) {
                _focus = FocusTarget::TopMenu;
                break;
            }
        }
        for (std::size_t index = 0; index < _subMenu.size(); ++index) {
            if (auto button = _subMenu.getButton(index); button && button->isFocused()) {
                _focus = FocusTarget::SubMenu;
                break;
            }
        }
    }
}

void SettingsPanel::updateVisuals(float deltaTime) {
    _topMenu.updateVisuals(deltaTime);
    _subMenu.updateVisuals(deltaTime);
}

void SettingsPanel::render(sf::RenderTarget& target) {
    const sf::Vector2f viewSize = target.getView().getSize();
    const sf::Vector2f topLeft = target.getView().getCenter() - viewSize * 0.5f;
    sf::RectangleShape backdrop(viewSize);
    backdrop.setPosition(topLeft);
    backdrop.setFillColor(sf::Color(0, 0, 0, 170));
    target.draw(backdrop);

    const sf::Font& font = ResourceManager::getInstance().getFont("SuperMario");
    sf::Text title(font, "SETTINGS", 48);
    title.setOutlineThickness(5.f);
    title.setOutlineColor(sf::Color::Black);
    title.setFillColor(sf::Color::White);
    title.setStyle(sf::Text::Bold);
    title.setPosition({960.f - title.getLocalBounds().size.x * 0.5f, 90.f});
    target.draw(title);
    _topMenu.render(target);
    _subMenu.render(target);

    const std::string hintText = _rebindingAction
        ? "PRESS ANY KEY TO REBIND (ESC TO CANCEL)"
        : "Left/Right switch tab | Up/Down navigate | Enter select | ESC back";
    sf::Text hint(font, hintText, 22);
    hint.setFillColor(_rebindingAction
        ? sf::Color(255, 180, 80) : sf::Color(230, 230, 230));
    hint.setPosition({960.f - hint.getLocalBounds().size.x * 0.5f, 965.f});
    target.draw(hint);
}

} // namespace UI
