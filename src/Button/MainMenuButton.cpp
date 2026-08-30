
#include "Button/MainMenuButton.h"
#include "Button/Button.h"
#include "ResourceManager.h"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Angle.hpp>

namespace UI {

MainMenuButton::MainMenuButton(const sf::Vector2f& position, const sf::Vector2f& size, const sf::Color& color, 
               const std::string& text, unsigned int charSize)
    : Button(position, size, color, text, charSize),
    triangle(10.f, 3) {
    defaultColor = sf::Color::White;
    focusedColor = sf::Color::Yellow;
    label.setFont(ResourceManager::getInstance().getFont("SuperMario"));
    label.setOutlineColor(sf::Color::Black);
    label.setOutlineThickness(2.0f);
    triangle.setFillColor(focusedColor);
    triangle.setRotation(sf::degrees(90.0f)); 
    triangle.setPosition({position.x, position.y + 20.0f});
    triangle.setOutlineColor(sf::Color::Black);
    triangle.setOutlineThickness(2.0f);
}

void MainMenuButton::render(sf::RenderTarget& target) {
    sf::Vector2f drawPos = basePosition;
    if (_isFocused) {
        drawPos.y -= liftAmount;
    }
    if (_isFocused) {
        label.setFillColor(sf::Color::Yellow);
    } else {
        label.setFillColor(sf::Color::White);
    }

    Button::updateLayout(drawPos, baseSize);

    if (!label.getString().isEmpty()) {
        target.draw(label);
    }
    if (_isFocused) {
        target.draw(triangle);
    }
}

void MainMenuButton::processEvent(const sf::Event& event) {
    // 1. HANDLE HOVER (POP-UP EFFECT)
    if (const auto* mouseMove = event.getIf<sf::Event::MouseMoved>()) {
        sf::Vector2f mousePos(static_cast<float>(mouseMove->position.x), static_cast<float>(mouseMove->position.y));
        // IMPORTANT: Check collision against the BASE position, NOT the current moving shape.
        // This prevents the button from jittering when the mouse is at the bottom edge.
        sf::FloatRect staticBounds{basePosition, baseSize};
        _isFocused = staticBounds.contains(mousePos); 

        if (_isFocused) {
            label.setFillColor(focusedColor);
        } else {
            label.setFillColor(defaultColor);
        }
    }

    // 2. HANDLE CLICK EVENT
    if (const auto* mouseEvent = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mouseEvent->button == sf::Mouse::Button::Left) {
            sf::Vector2f mousePos(static_cast<float>(mouseEvent->position.x), static_cast<float>(mouseEvent->position.y));
            
            // Check click against the static bounds as well to be consistent
            sf::FloatRect staticBounds{basePosition, baseSize};
            if (staticBounds.contains(mousePos)) {
                if (buttonCommand) buttonCommand->execute();
            }
        }
    }
}

} // namespace UI