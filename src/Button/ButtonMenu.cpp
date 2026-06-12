#include "ButtonMenu.h"

namespace UI {

void ButtonMenu::setLayoutProperties(const sf::Vector2f& startPosition,
                                     const sf::Vector2f& buttonSize,
                                     float spacing,
                                     bool horizontal,
                                     const sf::Color& defaultColor,
                                     unsigned int defaultCharSize) {
    _layout.startPosition = startPosition;
    _layout.buttonSize = buttonSize;
    _layout.spacing = spacing;
    _layout.horizontal = horizontal;
    _layout.defaultColor = defaultColor;
    _layout.defaultCharSize = defaultCharSize;

    for (std::size_t i = 0; i < _buttonMenu.size(); ++i) {
        const float offset = static_cast<float>(i) * _layout.spacing;
        const sf::Vector2f position = _layout.horizontal
            ? sf::Vector2f(_layout.startPosition.x + offset, _layout.startPosition.y)
            : sf::Vector2f(_layout.startPosition.x, _layout.startPosition.y + offset);

        _buttonMenu[i]->setSize(_layout.buttonSize);
        _buttonMenu[i]->setPosition(position);
    }
}

void ButtonMenu::addButton(const std::shared_ptr<Button>& button) {
    if (!button) {
        return;
    }

    _buttonMenu.push_back(button);
    syncFocus();
}

void ButtonMenu::addButtonAuto(const std::string& text, std::unique_ptr<ICommand> command, const std::string& iconAlias) {
    addButtonAuto(text, _layout.defaultCharSize, std::move(command), _layout.defaultColor, iconAlias);
}

void ButtonMenu::addButtonAuto(const std::string& text, unsigned int charSize, 
                                std::unique_ptr<ICommand> command, 
                                const sf::Color& color, const std::string& iconAlias) {
    const float offset = static_cast<float>(_buttonMenu.size()) * _layout.spacing;
    const sf::Vector2f position = _layout.horizontal
        ? sf::Vector2f(_layout.startPosition.x + offset, _layout.startPosition.y)
        : sf::Vector2f(_layout.startPosition.x, _layout.startPosition.y + offset);

    // Pass the iconAlias to the Button constructor
    auto button = std::make_shared<Button>(
        position, _layout.buttonSize, color, text, charSize, 20.0f, iconAlias
    );
    button->setCommand(std::move(command));
    addButton(button);
}

void ButtonMenu::processEvent(const sf::Event& event) {
    for (const std::shared_ptr<Button>& button : _buttonMenu) {
        button->processEvent(event);
    }

    if (auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
        if (_buttonMenu.empty()) return;

        if (_layout.horizontal) {
            if (keyEvent->code == sf::Keyboard::Key::Left || keyEvent->code == sf::Keyboard::Key::A) {
                _focusedIndex = (_focusedIndex - 1 + static_cast<int>(_buttonMenu.size())) % static_cast<int>(_buttonMenu.size());
                syncFocus();
            } else if (keyEvent->code == sf::Keyboard::Key::Right || keyEvent->code == sf::Keyboard::Key::D) {
                _focusedIndex = (_focusedIndex + 1) % static_cast<int>(_buttonMenu.size());
                syncFocus();
            }
        } else {
            if (keyEvent->code == sf::Keyboard::Key::Up || keyEvent->code == sf::Keyboard::Key::W) {
                _focusedIndex = (_focusedIndex - 1 + static_cast<int>(_buttonMenu.size())) % static_cast<int>(_buttonMenu.size());
                syncFocus();
            } else if (keyEvent->code == sf::Keyboard::Key::Down || keyEvent->code == sf::Keyboard::Key::S) {
                _focusedIndex = (_focusedIndex + 1) % static_cast<int>(_buttonMenu.size());
                syncFocus();
            }
        }

        if (keyEvent->code == sf::Keyboard::Key::Enter || keyEvent->code == sf::Keyboard::Key::Space) {
            _buttonMenu[_focusedIndex]->execute();
        }
    }

    if (event.is<sf::Event::MouseMoved>()) {
        for (std::size_t i = 0; i < _buttonMenu.size(); ++i) {
            if (_buttonMenu[i]->isHovered()) {
                _focusedIndex = static_cast<int>(i);
                syncFocus();
                break;
            }
        }
    }
}

void ButtonMenu::render(sf::RenderTarget& target) {
    for (const std::shared_ptr<Button>& button : _buttonMenu) {
        button->render(target);
    }
}

void ButtonMenu::clear() {
    _buttonMenu.clear();
}

std::size_t ButtonMenu::size() const {
    return _buttonMenu.size();
}

void ButtonMenu::syncFocus() {
    for (std::size_t i = 0; i < _buttonMenu.size(); ++i) {
        _buttonMenu[i]->setFocused(static_cast<int>(i) == _focusedIndex);
    }
}

void ButtonMenu::setFocusedIndex(int index) {
    if (index >= 0 && index < static_cast<int>(_buttonMenu.size())) {
        _focusedIndex = index;
        syncFocus();
    }
}

} // namespace UI
