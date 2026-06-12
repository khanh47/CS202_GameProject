#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <functional>
#include "ICommand.h"
#include <optional>

namespace UI {

class Button {
public:
    Button(const sf::Vector2f& position, const sf::Vector2f& size, const sf::Color& color, 
           const std::string& text, unsigned int charSize, 
           float cornerRadius = 10.0f, const std::string& iconAlias = "");

    void setCommand(std::unique_ptr<ICommand> command);
    void execute();
    void setPosition(const sf::Vector2f& position);
    void setSize(const sf::Vector2f& size);
    void setFocused(bool focused);
    bool isHovered() const { return _isHovered; }
    void processEvent(const sf::Event& event);
    void render(sf::RenderTarget& target);

private:

    sf::ConvexShape shape;
    sf::Text label;
    std::optional<sf::Sprite> icon;
    bool hasIcon = false;
    float cornerRadius;

    std::unique_ptr<ICommand> buttonCommand;

    sf::Color defaultColor;
    sf::Color hoverColor;
    sf::Color focusedColor;
    bool _isHovered = false;
    bool _isFocused = false;

    sf::Vector2f basePosition;
    sf::Vector2f baseSize;
    const float liftAmount = 3.0f;

    void updateRoundedShape(const sf::Vector2f& position, const sf::Vector2f& size);
    void updateLayout(const sf::Vector2f& position, const sf::Vector2f& size);
};
}
