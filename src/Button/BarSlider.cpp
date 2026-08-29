#include "Button/BarSlider.h"
#include <algorithm>
#include <cmath>

namespace UI {

BarSlider::BarSlider(const sf::Vector2f& position, const sf::Vector2f& size, const sf::Color& color,
                                                     const std::string& labelText, unsigned int charSize,
                                                     bool initialState, float cornerRadius) 
        : Button(position, size, initialState ? sf::Color(46, 204, 113) : sf::Color(108, 122, 137),
                         labelText, charSize, cornerRadius),
            _isSelected(initialState),
            _labelText(labelText) {
        (void)color;
        updateColors();
}

BarSlider::BarSlider(const sf::Vector2f& position, const sf::Vector2f& size, const sf::Color& color,
                                                     const std::string& labelText, unsigned int charSize,
                                                     float initialValue, float minValue, float maxValue,
                                                     bool selected, float cornerRadius)
        : Button(position, size, color, labelText, charSize, cornerRadius),
            _isSelected(selected), _labelText(labelText), _value(initialValue), _minValue(minValue), _maxValue(maxValue) {
        setText(_labelText);
        updateColors();
}

void BarSlider::setSelected(bool selected) {
    _isSelected = selected;
    updateColors();
}

void BarSlider::select() {
    _isSelected = !_isSelected;
    updateColors();
}

void BarSlider::setSelectCallback(SelectCallback callback) {
    _selectCallback = std::move(callback);
}

void BarSlider::execute() {
    select();
    if (_selectCallback) {
        _selectCallback(_isSelected);
    }
    Button::execute();
}

void BarSlider::updateColors() {
    sf::Color base = _isSelected ? _onColor : _offColor;
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
    // setText(_labelText);
}

void BarSlider::render(sf::RenderTarget& target) {
    // Render only the base button and label. Track/knob visuals and mouse drag
    // interactions are intentionally removed so this control acts as an
    // invisible numeric value adjusted via keyboard.
    Button::render(target);
}

void BarSlider::setValue(float value) {
    float clamped = std::clamp(value, _minValue, _maxValue);
    if (std::abs(clamped - _value) > 1e-6f) {
        _value = clamped;
        if (_valueCallback) {
            _valueCallback(_value);
        }
    }
}

float BarSlider::getValue() const {
    return _value;
}

void BarSlider::setValueCallback(ValueCallback cb) {
    _valueCallback = std::move(cb);
}

void BarSlider::adjust(float delta) {
    setValue(_value + delta);
}

} // namespace UI
