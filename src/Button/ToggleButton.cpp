#include "Button/ToggleButton.h"
#include <algorithm>
#include <cmath>

namespace UI {

ToggleButton::ToggleButton(const sf::Vector2f& position, const sf::Vector2f& size, const sf::Color& color,
                           const std::string& labelText, unsigned int charSize,
                           bool initialState, float cornerRadius)
    : Button(position, size, initialState ? sf::Color(46, 204, 113) : sf::Color(108, 122, 137),
             labelText, charSize, cornerRadius),
      _isToggled(initialState),
      _labelText(labelText) {
    (void)color;
    updateColors();
}

void ToggleButton::setToggled(bool toggled) {
    _isToggled = toggled;
    updateColors();
}

void ToggleButton::toggle() {
    _isToggled = !_isToggled;
    updateColors();
}

void ToggleButton::setToggleCallback(ToggleCallback callback) {
    _toggleCallback = std::move(callback);
}

void ToggleButton::execute() {
    toggle();
    if (_toggleCallback) {
        _toggleCallback(_isToggled);
    }
    Button::execute();
}

void ToggleButton::updateColors() {
    sf::Color base = _isToggled ? _onColor : _offColor;
    defaultColor = base;

    int r = std::min(255, base.r + 35);
    int g = std::min(255, base.g + 35);
    int b = std::min(255, base.b + 35);
    focusedColor = sf::Color(r, g, b, base.a);

    int fr = std::min(255, base.r + 65);
    int fg = std::min(255, base.g + 65);
    int fb = std::min(255, base.b + 65);
    focusedColor = sf::Color(fr, fg, fb, base.a);

    shape.setFillColor(defaultColor);
    setText(_labelText);
}

void ToggleButton::render(sf::RenderTarget& target) {
    // 1. Render base button background shape and clean label text
    Button::render(target);

    // Calculate vertical hover displacement for precise slider alignment
    sf::Vector2f drawPos = basePosition;
    if (_isFocused) {
        drawPos.y -= liftAmount;
    }

    // 2. Render Rounded Pill Slider Track (Fully curved capsule shape on the right edge)
    const float trackWidth = 44.0f;
    const float trackHeight = 22.0f;
    const float paddingRight = 18.0f;

    const float trackX = drawPos.x + baseSize.x - trackWidth - paddingRight;
    const float trackY = drawPos.y + (baseSize.y - trackHeight) / 2.0f;

    sf::ConvexShape track;
    const int pointsPerCorner = 8;
    track.setPointCount(pointsPerCorner * 4);

    const float radius = trackHeight / 2.0f; // Fully rounded semicircular caps
    const float pi = 3.141592654f;
    int pointIndex = 0;

    auto addCorner = [&](float cx, float cy, float startAngle) {
        for (int i = 0; i < pointsPerCorner; ++i) {
            float angle = startAngle + (i * (pi / 2.0f) / (pointsPerCorner - 1));
            float px = cx + radius * std::cos(angle);
            float py = cy + radius * std::sin(angle);
            track.setPoint(pointIndex++, sf::Vector2f(px, py));
        }
    };

    addCorner(trackWidth - radius, trackHeight - radius, 0.0f);
    addCorner(radius, trackHeight - radius, pi / 2.0f);
    addCorner(radius, radius, pi);
    addCorner(trackWidth - radius, radius, 3.0f * pi / 2.0f);

    track.setPosition({trackX, trackY});

    if (_isToggled) {
        track.setFillColor(sf::Color(30, 130, 70)); // Dark active green track
    } else {
        track.setFillColor(sf::Color(75, 85, 95));   // Dark inactive gray track
    }
    track.setOutlineThickness(1.5f);
    track.setOutlineColor(sf::Color(255, 255, 255, 180));
    target.draw(track);

    // 3. Render Slider Knob (Circular thumb indicator inside pill track)
    const float knobRadius = 8.0f;
    sf::CircleShape knob(knobRadius);
    const float knobY = trackY + (trackHeight - knobRadius * 2.0f) / 2.0f;

    float knobX = 0.0f;
    if (_isToggled) {
        knobX = trackX + trackWidth - knobRadius * 2.0f - 3.0f; // Right position when ON
    } else {
        knobX = trackX + 3.0f;                                  // Left position when OFF
    }

    knob.setPosition({knobX, knobY});
    knob.setFillColor(sf::Color::White);
    target.draw(knob);
}

} // namespace UI
