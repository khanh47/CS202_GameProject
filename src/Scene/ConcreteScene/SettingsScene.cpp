#include "Scene/ConcreteScene/SettingsScene.h"
#include "Button/BarSlider.h"
#include "ResourceManager.h"
#include "Scene/SceneManager.h"
#include "Button/ToggleButton.h"
#include "Commands/ToggleDebugCommands.h"
#include "Commands/FunctionalCommand.h"
#include "Game/GameSettings.h"

#include <algorithm>
#include <string>

SettingsScene::SettingsScene()
    : Scene("SettingsScene") {
    
}

void SettingsScene::init() {
    buildTopMenu();
    setActiveTab(SettingsTab::Music);
}

void SettingsScene::onEnter() {
    Scene::onEnter();
    updateKeybindButtonTexts();
    updateVolumeLabel();
    if (_activeTab == SettingsTab::Music) {
        auto& settings = GameSettings::getInstance();
        if (_btnMusicVolume) {
            _btnMusicVolume->setValue(settings.musicVolume);
        }
        if (_btnSoundVolume) {
            _btnSoundVolume->setValue(settings.soundVolume);
        }
    }
}

void SettingsScene::onExit() {
    Scene::onExit();
    _rebindingAction.reset();
    _activeRebindingButton.reset();
}

void SettingsScene::buildTopMenu() {
    _topMenu.clear();
    _topMenu.setLayoutProperties({515.f, 180.f}, {200.f, 60.f}, 230.f, true, sf::Color(70, 130, 180), 26);

    _topMenu.addButtonAuto("Music", std::make_unique<FunctionalCommand>("TabMusic", [this]() {
        setActiveTab(SettingsTab::Music);
    }));
    _topMenu.addButtonAuto("Keybind", std::make_unique<FunctionalCommand>("TabKeybind", [this]() {
        setActiveTab(SettingsTab::Keybind);
    }));
    _topMenu.addButtonAuto("Dev Mode", std::make_unique<FunctionalCommand>("TabDevMode", [this]() {
        setActiveTab(SettingsTab::DevMode);
    }));
    _topMenu.addButtonAuto("Back", 26, std::make_unique<FunctionalCommand>("BackToMain", [this]() {
        if (auto mgr = getSceneManager()) {
            mgr->requestPopScene();
        }
    }), sf::Color(180, 80, 80));
}

void SettingsScene::buildMusicPanel() {
    const sf::Vector2f startPos{780.f, 340.f};
    const sf::Vector2f btnSize{360.f, 60.f};
    const float spacing = 80.f;
    const sf::Color btnColor(70, 130, 180);
    _subMenu.setLayoutProperties(startPos, btnSize, spacing, false, btnColor, 24);

    auto& settings = GameSettings::getInstance();

    auto musicToggle = std::make_shared<UI::ToggleButton>(
        sf::Vector2f(startPos.x, startPos.y + 0.f * spacing), btnSize, btnColor,
        "Music Enabled", 24, settings.musicEnabled, 20.0f
    );
    musicToggle->setToggleCallback([](bool on) {
        GameSettings::getInstance().musicEnabled = on;
    });
    _subMenu.addButton(musicToggle);

    _btnMusicVolume = std::make_shared<UI::BarSlider>(
        sf::Vector2f(startPos.x, startPos.y + 1.f * spacing), btnSize, btnColor,
        "Volume", 24, settings.musicVolume, 0.f, 100.f, false, 20.0f
    );
    _btnMusicVolume->setValueCallback([this](float value) {
        auto& s = GameSettings::getInstance();
        s.musicVolume = std::clamp(value, 0.f, 100.f);
        updateVolumeLabel();
        refreshTitleScreenMusicVolume();
    });
    _subMenu.addButton(_btnMusicVolume);

    auto soundToggle = std::make_shared<UI::ToggleButton>(
        sf::Vector2f(startPos.x, startPos.y + 2.f * spacing), btnSize, btnColor,
        "Sound Enabled", 24, settings.soundEnabled, 20.0f
    );
    soundToggle->setToggleCallback([](bool on) {
        GameSettings::getInstance().soundEnabled = on;
    });
    _subMenu.addButton(soundToggle);

    _btnSoundVolume = std::make_shared<UI::BarSlider>(
        sf::Vector2f(startPos.x, startPos.y + 3.f * spacing), btnSize, btnColor,
        "Sound", 24, settings.soundVolume, 0.f, 100.f, false, 20.0f
    );
    _btnSoundVolume->setValueCallback([this](float value) {
        auto& s = GameSettings::getInstance();
        s.soundVolume = std::clamp(value, 0.f, 100.f);
        updateVolumeLabel();
    });
    _subMenu.addButton(_btnSoundVolume);

    updateVolumeLabel();
}

void SettingsScene::buildKeybindPanel() {
    const sf::Vector2f startPos{780.f, 340.f};
    const sf::Vector2f btnSize{360.f, 60.f};
    const float spacing = 80.f;
    const sf::Color btnColor(70, 130, 180);

    _subMenu.setLayoutProperties(startPos, btnSize, spacing, false, btnColor, 24);

    auto& settings = GameSettings::getInstance();

    _btnMoveLeft = std::make_shared<UI::Button>(
        sf::Vector2f(startPos.x, startPos.y + 0.f * spacing), btnSize, btnColor,
        "Move Left: " + GameSettings::keyToString(settings.keyMoveLeft), 24, 20.0f
    );
    _btnMoveLeft->setCommand(std::make_unique<FunctionalCommand>("RebindLeft", [this]() {
        startRebinding(ActionType::MoveLeft, _btnMoveLeft);
    }));
    _subMenu.addButton(_btnMoveLeft);

    _btnMoveRight = std::make_shared<UI::Button>(
        sf::Vector2f(startPos.x, startPos.y + 1.f * spacing), btnSize, btnColor,
        "Move Right: " + GameSettings::keyToString(settings.keyMoveRight), 24, 20.0f
    );
    _btnMoveRight->setCommand(std::make_unique<FunctionalCommand>("RebindRight", [this]() {
        startRebinding(ActionType::MoveRight, _btnMoveRight);
    }));
    _subMenu.addButton(_btnMoveRight);

    _btnJump = std::make_shared<UI::Button>(
        sf::Vector2f(startPos.x, startPos.y + 2.f * spacing), btnSize, btnColor,
        "Jump: " + GameSettings::keyToString(settings.keyJump), 24, 20.0f
    );
    _btnJump->setCommand(std::make_unique<FunctionalCommand>("RebindJump", [this]() {
        startRebinding(ActionType::MoveUp, _btnJump);
    }));
    _subMenu.addButton(_btnJump);

    _btnAttack = std::make_shared<UI::Button>(
        sf::Vector2f(startPos.x, startPos.y + 3.f * spacing), btnSize, btnColor,
        "Shoot: " + GameSettings::keyToString(settings.keyAttack), 24, 20.0f
    );
    _btnAttack->setCommand(std::make_unique<FunctionalCommand>("RebindAttack", [this]() {
        startRebinding(ActionType::Attack, _btnAttack);
    }));
    _subMenu.addButton(_btnAttack);

    _btnButton = std::make_shared<UI::Button>(
        sf::Vector2f(startPos.x, startPos.y + 4.f * spacing), btnSize, btnColor,
        "Button: " + GameSettings::keyToString(settings.keyInteract), 24, 20.0f
    );
    _btnButton->setCommand(std::make_unique<FunctionalCommand>("RebindButton", [this]() {
        startRebinding(ActionType::Interact, _btnButton);
    }));
    _subMenu.addButton(_btnButton);
}

void SettingsScene::buildDevModePanel() {
    const sf::Vector2f startPos{780.f, 340.f};
    const sf::Vector2f btnSize{360.f, 60.f};
    const float spacing = 80.f;

    _subMenu.setLayoutProperties(startPos, btnSize, spacing, false, sf::Color(100, 149, 237), 24);

    auto& settings = GameSettings::getInstance();

    _subMenu.addToggleButtonAuto("Grid", settings.debugDrawGrid, std::make_unique<ToggleGridCommand>());
    _subMenu.addToggleButtonAuto("Coordinates", settings.debugDrawCoordinates, std::make_unique<ToggleCoordinatesCommand>());
    _subMenu.addToggleButtonAuto("Hitbox", settings.debugDrawHitbox, std::make_unique<ToggleHitboxCommand>());
    _subMenu.addToggleButtonAuto("Camera Move", settings.freeCameraMove, std::make_unique<ToggleFreeCameraCommand>());
}

void SettingsScene::setActiveTab(SettingsTab tab) {
    _activeTab = tab;
    _subMenu.clear();
    switch (tab) {
        case SettingsTab::Music:
            buildMusicPanel();
            break;
        case SettingsTab::Keybind:
            buildKeybindPanel();
            updateKeybindButtonTexts();
            break;
        case SettingsTab::DevMode:
            buildDevModePanel();
            break;
    }
    _subMenu.setFocusedIndex(0);
    _focus = FocusTarget::SubMenu;
}

void SettingsScene::startRebinding(ActionType action, std::shared_ptr<UI::Button> button) {
    _rebindingAction = action;
    _activeRebindingButton = button;
    if (_activeRebindingButton) {
        _activeRebindingButton->setText("< Press Key >");
    }
}

void SettingsScene::updateKeybindButtonTexts() {
    auto& settings = GameSettings::getInstance();
    if (_btnMoveLeft) {
        _btnMoveLeft->setText("Move Left: " + GameSettings::keyToString(settings.keyMoveLeft));
    }
    if (_btnMoveRight) {
        _btnMoveRight->setText("Move Right: " + GameSettings::keyToString(settings.keyMoveRight));
    }
    if (_btnJump) {
        _btnJump->setText("Jump: " + GameSettings::keyToString(settings.keyJump));
    }
    if (_btnAttack) {
        _btnAttack->setText("Shoot: " + GameSettings::keyToString(settings.keyAttack));
    }
    if (_btnButton) {
        _btnButton->setText("Button: " + GameSettings::keyToString(settings.keyInteract));
    }
}

void SettingsScene::updateVolumeLabel() {
    if (_btnMusicVolume) {
        _btnMusicVolume->setText("Volume: " + std::to_string(static_cast<int>(GameSettings::getInstance().musicVolume)) + "%");
    }
    if (_btnSoundVolume) {
        _btnSoundVolume->setText("Sound: " + std::to_string(static_cast<int>(GameSettings::getInstance().soundVolume)) + "%");
    }
}

void SettingsScene::navigateMenu(UI::ButtonMenu& menu, int delta) {
    const int count = static_cast<int>(menu.size());
    if (count <= 0) {
        return;
    }
    menu.setFocusedIndex((menu.getFocusedIndex() + delta + count) % count);
}

void SettingsScene::handleInput(const sf::Event& event) {
    // 1. Rebinding capture (Keybind panel)
    if (_rebindingAction.has_value()) {
        if (auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
            if (keyEvent->code == sf::Keyboard::Key::Escape) {
                updateKeybindButtonTexts();
                _rebindingAction.reset();
                _activeRebindingButton.reset();
                return;
            }
            if (keyEvent->code != sf::Keyboard::Key::Unknown) {
                GameSettings::getInstance().setKeyForAction(*_rebindingAction, keyEvent->code);
                updateKeybindButtonTexts();
                _rebindingAction.reset();
                _activeRebindingButton.reset();
                return;
            }
        }
        return;
    }

    // 2. Keyboard navigation
    if (auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
        if (_focus == FocusTarget::SubMenu && _activeTab == SettingsTab::Music) {
            const auto focusedButton = _subMenu.getButton(static_cast<std::size_t>(_subMenu.getFocusedIndex()));
            if (auto slider = std::dynamic_pointer_cast<UI::BarSlider>(focusedButton)) {
                if (slider->isSelected()) {
                    if (keyEvent->code == sf::Keyboard::Key::Left || keyEvent->code == sf::Keyboard::Key::A) {
                        slider->adjust(-5.f);
                        return;
                    }
                    if (keyEvent->code == sf::Keyboard::Key::Right || keyEvent->code == sf::Keyboard::Key::D) {
                        slider->adjust(5.f);
                        return;
                    }
                }
            }
        }

        switch (keyEvent->code) {
            case sf::Keyboard::Key::Escape:
                if (auto mgr = getSceneManager()) {
                    mgr->requestPopScene();
                }
                return;
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
                UI::ButtonMenu& menu = (_focus == FocusTarget::TopMenu) ? _topMenu : _subMenu;
                if (auto button = menu.getButton(menu.getFocusedIndex())) {
                    button->execute();
                }
                return;
            }
            default:
                return;
        }
    }

    // 3. Mouse events: top and sub menus are both interactable
    _topMenu.processEvent(event);
    _subMenu.processEvent(event);

    if (event.is<sf::Event::MouseMoved>()) {
        for (std::size_t i = 0; i < _topMenu.size(); ++i) {
            if (auto button = _topMenu.getButton(i); button && button->isHovered()) {
                _focus = FocusTarget::TopMenu;
                break;
            }
        }
        for (std::size_t i = 0; i < _subMenu.size(); ++i) {
            if (auto button = _subMenu.getButton(i); button && button->isHovered()) {
                _focus = FocusTarget::SubMenu;
                break;
            }
        }
    }
}

void SettingsScene::updateVisuals(float deltaTime) {
    _topMenu.updateVisuals(deltaTime);
    _subMenu.updateVisuals(deltaTime);
}

void SettingsScene::render(sf::RenderTarget& target) {
    const sf::Vector2f viewSize = target.getView().getSize();
    const sf::Vector2f viewPosition = target.getView().getCenter() - viewSize * 0.5f;

    // Translucent backdrop over the blurred scene for readability
    sf::RectangleShape backdrop(viewSize);
    backdrop.setPosition(viewPosition);
    backdrop.setFillColor(sf::Color(0, 0, 0, 120));
    target.draw(backdrop);

    const sf::Font& font = ResourceManager::getInstance().getFont("SuperMario");

    sf::Text title(font, "SETTINGS", 48);
    title.setOutlineThickness(5.0f);
    title.setOutlineColor(sf::Color::Black);
    title.setFillColor(sf::Color::White);
    title.setStyle(sf::Text::Bold);
    title.setPosition({1920.f / 2.f - title.getLocalBounds().size.x / 2.f, 90.f});
    target.draw(title);

    _topMenu.render(target);
    _subMenu.render(target);

    // Footer hint
    std::string hintStr = _rebindingAction.has_value()
        ? "PRESS ANY KEY TO REBIND (ESC TO CANCEL)"
        : "Left/Right switch tab | Up/Down navigate | Enter select | ESC back";

    sf::Text hint(font, hintStr, 22);
    hint.setFillColor(_rebindingAction.has_value() ? sf::Color(220, 100, 0) : sf::Color(230, 230, 230));
    hint.setPosition({1920.f / 2.f - hint.getLocalBounds().size.x / 2.f, 850.f});
    target.draw(hint);
}
