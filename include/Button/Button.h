#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include "Commands/ICommand.h"
#include <optional>

namespace UI {

class Button {
public:
    Button(const sf::Vector2f& position, const sf::Vector2f& size, const sf::Color& color, 
           const std::string& text, unsigned int charSize, 
           float cornerRadius = 10.0f, const std::string& iconAlias = "");
    virtual ~Button() = default;

    void setCommand(std::unique_ptr<ICommand> command);
    virtual void execute();
    void setPosition(const sf::Vector2f& position);
    void setSize(const sf::Vector2f& size);
    void setFocused(bool focused);
    void clearHover() noexcept { _isHovered = false; }
    bool isHovered() const { return _isHovered; }
    virtual void processEvent(const sf::Event& event);
    virtual void updateVisuals(float deltaTime) { (void)deltaTime; }
    virtual void render(sf::RenderTarget& target);

    void setText(const std::string& text);
    std::string getText() const;

protected:
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
