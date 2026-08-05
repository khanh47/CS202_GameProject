#include "Scene/ConcreteScene/KeyBindSettingsScene.h"
#include "ResourceManager.h"
#include "Scene/SceneManager.h"
#include "Commands/FunctionalCommand.h"
#include "Game/GameSettings.h"

KeyBindSettingsScene::KeyBindSettingsScene()
    : _name("KeyBindSettingsScene") {
}

void KeyBindSettingsScene::init() {
    auto& settings = GameSettings::getInstance();

    const sf::Vector2f startPos{1920.f / 2.f - 180.f, 300.f};
    const sf::Vector2f btnSize{360.f, 60.f};
    const float spacing = 80.f;
    const sf::Color btnColor(70, 130, 180);

    _menu.setLayoutProperties(startPos, btnSize, spacing, false, btnColor, 24);

    _btnMoveLeft = std::make_shared<UI::Button>(
        sf::Vector2f(startPos.x, startPos.y + 0.f * spacing), btnSize, btnColor,
        "Move Left: " + GameSettings::keyToString(settings.keyMoveLeft), 24, 20.0f
    );
    _btnMoveLeft->setCommand(std::make_unique<FunctionalCommand>("RebindLeft", [this]() {
        this->startRebinding(ActionType::MoveLeft, _btnMoveLeft);
    }));
    _menu.addButton(_btnMoveLeft);

    _btnMoveRight = std::make_shared<UI::Button>(
        sf::Vector2f(startPos.x, startPos.y + 1.f * spacing), btnSize, btnColor,
        "Move Right: " + GameSettings::keyToString(settings.keyMoveRight), 24, 20.0f
    );
    _btnMoveRight->setCommand(std::make_unique<FunctionalCommand>("RebindRight", [this]() {
        this->startRebinding(ActionType::MoveRight, _btnMoveRight);
    }));
    _menu.addButton(_btnMoveRight);

    _btnJump = std::make_shared<UI::Button>(
        sf::Vector2f(startPos.x, startPos.y + 2.f * spacing), btnSize, btnColor,
        "Jump: " + GameSettings::keyToString(settings.keyJump), 24, 20.0f
    );
    _btnJump->setCommand(std::make_unique<FunctionalCommand>("RebindJump", [this]() {
        this->startRebinding(ActionType::MoveUp, _btnJump);
    }));
    _menu.addButton(_btnJump);

    _btnAttack = std::make_shared<UI::Button>(
        sf::Vector2f(startPos.x, startPos.y + 3.f * spacing), btnSize, btnColor,
        "Shoot: " + GameSettings::keyToString(settings.keyAttack), 24, 20.0f
    );
    _btnAttack->setCommand(std::make_unique<FunctionalCommand>("RebindAttack", [this]() {
        this->startRebinding(ActionType::Attack, _btnAttack);
    }));
    _menu.addButton(_btnAttack);

    _btnBack = std::make_shared<UI::Button>(
        sf::Vector2f(startPos.x, startPos.y + 4.f * spacing), btnSize, sf::Color(180, 80, 80),
        "Back", 24, 20.0f
    );
    _btnBack->setCommand(std::make_unique<FunctionalCommand>("BackToSettings", [this]() {
        if (auto mgr = getSceneManager()) {
            mgr->requestPopScene();
        }
    }));
    _menu.addButton(_btnBack);
}

void KeyBindSettingsScene::onEnter() {
    _isActive = true;
    updateKeybindButtonTexts();
}

void KeyBindSettingsScene::onExit() {
    _isActive = false;
    _rebindingAction.reset();
    _activeRebindingButton.reset();
}

void KeyBindSettingsScene::cleanup() {
}

void KeyBindSettingsScene::startRebinding(ActionType action, std::shared_ptr<UI::Button> button) {
    _rebindingAction = action;
    _activeRebindingButton = button;
    if (_activeRebindingButton) {
        _activeRebindingButton->setText("< Press Key >");
    }
}

void KeyBindSettingsScene::updateKeybindButtonTexts() {
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
}

void KeyBindSettingsScene::handleInput(const sf::Event& event) {
    if (_rebindingAction.has_value()) {
        if (auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
            if (keyEvent->code == sf::Keyboard::Key::Escape) {
                // Cancel rebinding
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

    if (auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
        if (keyEvent->code == sf::Keyboard::Key::Escape) {
            if (auto mgr = getSceneManager()) {
                mgr->requestPopScene();
                return;
            }
        }
    }
    _menu.processEvent(event);
}

void KeyBindSettingsScene::updateVisuals(float deltaTime) {
    _menu.updateVisuals(deltaTime);
}

void KeyBindSettingsScene::render(sf::RenderTarget& target) {
    const sf::Font& font = ResourceManager::getInstance().getFont("Roboto");

    // Title Header
    sf::Text title(font, "KEYBIND SETTINGS", 48);
    title.setFillColor(sf::Color::Black);
    title.setStyle(sf::Text::Bold);
    title.setPosition({1920.f / 2.f - title.getLocalBounds().size.x / 2.f, 160.f});
    target.draw(title);

    _menu.render(target);

    // Footer Hint
    std::string hintStr = _rebindingAction.has_value()
        ? "PRESS ANY KEY TO REBIND (ESC TO CANCEL)"
        : "Click a button to change keybind | Press ESC to Back";

    sf::Text hint(font, hintStr, 22);
    hint.setFillColor(_rebindingAction.has_value() ? sf::Color(220, 100, 0) : sf::Color(100, 100, 100));
    hint.setPosition({1920.f / 2.f - hint.getLocalBounds().size.x / 2.f, 750.f});
    target.draw(hint);
}
