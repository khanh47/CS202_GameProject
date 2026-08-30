#include "Game/Camera.h"
#include "Game/Objects/GameObject.h"
#include "Game/GameSettings.h"

#include <cmath>
#include <algorithm>

Camera::Camera() : Camera(sf::Vector2f(1920.0f, 1080.0f)) {
}

Camera::Camera(const sf::Vector2f& size) {
    _view.setSize(size);
    _currentCenter = {size.x / 2.0f, size.y / 2.0f};
    _view.setCenter(_currentCenter);
}

void Camera::update(float deltaTime) {
    // Clean up expired targets
    if (_target && _target->isPendingDestroy()) {
        _target.reset();
    }
    _targets.erase(
        std::remove_if(_targets.begin(), _targets.end(), [](const auto& t) {
            return !t || t->isPendingDestroy();
        }),
        _targets.end()
    );

    if (deltaTime <= 0.0f) {
        return;
    }

    if (GameSettings::getInstance().freeCameraMove) {
        // Free camera movement (manual WASD / Arrow keys control)
        float offsetX = 0.0f;
        float offsetY = 0.0f;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
            offsetX -= _freeMoveSpeed * deltaTime;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
            offsetX += _freeMoveSpeed * deltaTime;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
            offsetY -= _freeMoveSpeed * deltaTime;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
            offsetY += _freeMoveSpeed * deltaTime;
        }

        _currentCenter += sf::Vector2f(offsetX, offsetY);
        _currentCenter = clampToBounds(_currentCenter);
        _view.setCenter(_currentCenter);
    } else if (_targets.size() > 1) {
        // Multi-Target Framing (Co-op Mode)
        float minX = 1e9f, maxX = -1e9f;
        float minY = 1e9f, maxY = -1e9f;
        sf::Vector2f avgVel{0.0f, 0.0f};

        for (const auto& t : _targets) {
            const sf::Vector2f pos = t->getPosition();
            const sf::Vector2f vel = t->getVelocity();
            minX = std::min(minX, pos.x);
            maxX = std::max(maxX, pos.x);
            minY = std::min(minY, pos.y);
            maxY = std::max(maxY, pos.y);
            avgVel += vel;
        }
        avgVel /= static_cast<float>(_targets.size());

        const sf::Vector2f targetPos{(minX + maxX) * 0.5f, (minY + maxY) * 0.5f};

        // Smooth Dynamic Zoom: scale viewport slightly if players move apart
        const float spanX = (maxX - minX) + 400.0f;
        const float spanY = (maxY - minY) + 300.0f;
        const float zoomFactor = std::clamp(
            std::max(spanX / _baseSize.x, spanY / _baseSize.y),
            1.0f, 1.25f
        );
        const sf::Vector2f desiredViewSize = _baseSize * zoomFactor;
        const sf::Vector2f currentSize = _view.getSize();
        const float sizeFactor = 1.0f - std::exp(-3.0f * deltaTime);
        _view.setSize(currentSize + (desiredViewSize - currentSize) * sizeFactor);

        sf::Vector2f targetFocus = calculateTargetFocus(targetPos, avgVel);

        float targetLookahead = 0.0f;
        if (avgVel.x > 10.0f) {
            targetLookahead = _config.lookaheadDistance * 0.75f;
        } else if (avgVel.x < -10.0f) {
            targetLookahead = -_config.lookaheadDistance * 0.75f;
        }

        const float lookaheadFactor = 1.0f - std::exp(-_config.lookaheadSpeed * deltaTime);
        _currentLookaheadX += (targetLookahead - _currentLookaheadX) * lookaheadFactor;
        targetFocus.x += _currentLookaheadX;

        const float factorX = 1.0f - std::exp(-_config.dampingX * deltaTime);
        const float factorY = 1.0f - std::exp(-_config.dampingY * deltaTime);

        _currentCenter.x += (targetFocus.x - _currentCenter.x) * factorX;
        _currentCenter.y += (targetFocus.y - _currentCenter.y) * factorY;

        _currentCenter = clampToBounds(_currentCenter);
        _view.setCenter({std::round(_currentCenter.x), std::round(_currentCenter.y)});
    } else if (_target || !_targets.empty()) {
        auto currentTarget = _target ? _target : _targets.front();
        
        // Smoothly restore base view size if previously zoomed
        const sf::Vector2f currentSize = _view.getSize();
        if (std::abs(currentSize.x - _baseSize.x) > 1.0f || std::abs(currentSize.y - _baseSize.y) > 1.0f) {
            const float sizeFactor = 1.0f - std::exp(-4.0f * deltaTime);
            _view.setSize(currentSize + (_baseSize - currentSize) * sizeFactor);
        }

        const sf::Vector2f targetPos = currentTarget->getPosition();
        const sf::Vector2f targetVel = currentTarget->getVelocity();

        sf::Vector2f targetFocus = calculateTargetFocus(targetPos, targetVel);

        float targetLookahead = 0.0f;
        if (targetVel.x > 10.0f) {
            targetLookahead = _config.lookaheadDistance;
        } else if (targetVel.x < -10.0f) {
            targetLookahead = -_config.lookaheadDistance;
        }

        const float lookaheadFactor = 1.0f - std::exp(-_config.lookaheadSpeed * deltaTime);
        _currentLookaheadX += (targetLookahead - _currentLookaheadX) * lookaheadFactor;
        targetFocus.x += _currentLookaheadX;

        const float factorX = 1.0f - std::exp(-_config.dampingX * deltaTime);
        const float factorY = 1.0f - std::exp(-_config.dampingY * deltaTime);

        _currentCenter.x += (targetFocus.x - _currentCenter.x) * factorX;
        _currentCenter.y += (targetFocus.y - _currentCenter.y) * factorY;

        _currentCenter = clampToBounds(_currentCenter);
        _view.setCenter({std::round(_currentCenter.x), std::round(_currentCenter.y)});
    }
}

sf::Vector2f Camera::calculateTargetFocus(const sf::Vector2f& targetPos, const sf::Vector2f& targetVel) const {
    (void)targetVel;
    sf::Vector2f focus = _currentCenter;

    const float dx = targetPos.x - _currentCenter.x;
    const float dy = targetPos.y - _currentCenter.y;
    const float halfW = _config.deadzoneSize.x * 0.5f;
    const float halfH = _config.deadzoneSize.y * 0.5f;

    // Horizontal Deadzone: Camera shifts only when player pushes against box boundaries
    if (dx > halfW) {
        focus.x = targetPos.x - halfW;
    } else if (dx < -halfW) {
        focus.x = targetPos.x + halfW;
    } else {
        focus.x = _currentCenter.x;
    }

    // Vertical Stabilization vs Deadzone: Ignores minor jumps unless vertical threshold is exceeded
    if (_config.yStabilizationEnabled) {
        if (std::abs(dy) > _config.yThreshold) {
            focus.y = targetPos.y - (dy > 0.0f ? _config.yThreshold : -_config.yThreshold);
        } else {
            focus.y = _currentCenter.y;
        }
    } else {
        if (dy > halfH) {
            focus.y = targetPos.y - halfH;
        } else if (dy < -halfH) {
            focus.y = targetPos.y + halfH;
        } else {
            focus.y = _currentCenter.y;
        }
    }

    return focus;
}

sf::Vector2f Camera::clampToBounds(const sf::Vector2f& center) const {
    if (!_config.useBounds) {
        return center;
    }

    const sf::Vector2f viewHalfSize = _view.getSize() * 0.5f;
    sf::Vector2f clamped = center;

    const float minX = _config.levelBounds.position.x + viewHalfSize.x;
    const float maxX = _config.levelBounds.position.x + _config.levelBounds.size.x - viewHalfSize.x;
    const float minY = _config.levelBounds.position.y + viewHalfSize.y;
    const float maxY = _config.levelBounds.position.y + _config.levelBounds.size.y - viewHalfSize.y;

    if (maxX >= minX) {
        clamped.x = std::clamp(clamped.x, minX, maxX);
    } else {
        // Level is narrower than view width: center view horizontally in level
        clamped.x = _config.levelBounds.position.x + _config.levelBounds.size.x * 0.5f;
    }

    if (maxY >= minY) {
        clamped.y = std::clamp(clamped.y, minY, maxY);
    } else {
        // Level is shorter than view height: center view vertically in level
        clamped.y = _config.levelBounds.position.y + _config.levelBounds.size.y * 0.5f;
    }

    return clamped;
}

void Camera::setTarget(std::shared_ptr<GameObject> target) {
    _target = target;
    _targets.clear();
    if (_target) {
        _targets.push_back(_target);
        // Center camera immediately on target when set
        _currentCenter = clampToBounds(_target->getPosition());
        _view.setCenter(_currentCenter);
    }
}

void Camera::setTargets(const std::vector<std::shared_ptr<GameObject>>& targets) {
    _targets.clear();
    for (const auto& t : targets) {
        if (t && !t->isPendingDestroy()) {
            _targets.push_back(t);
        }
    }
    if (_targets.empty()) {
        _target.reset();
    } else {
        _target = _targets.front();
        if (_targets.size() == 1) {
            _currentCenter = clampToBounds(_target->getPosition());
        } else {
            float minX = 1e9f, maxX = -1e9f, minY = 1e9f, maxY = -1e9f;
            for (const auto& t : _targets) {
                const sf::Vector2f p = t->getPosition();
                minX = std::min(minX, p.x);
                maxX = std::max(maxX, p.x);
                minY = std::min(minY, p.y);
                maxY = std::max(maxY, p.y);
            }
            _currentCenter = clampToBounds({(minX + maxX) * 0.5f, (minY + maxY) * 0.5f});
        }
        _view.setCenter(_currentCenter);
    }
}

void Camera::setConfig(const CameraConfig& config) {
    _config = config;
    _currentCenter = clampToBounds(_currentCenter);
    _view.setCenter(_currentCenter);
}

CameraConfig& Camera::getConfig() {
    return _config;
}

const CameraConfig& Camera::getConfig() const {
    return _config;
}

void Camera::setDeadzone(const sf::Vector2f& size) {
    _config.deadzoneSize = size;
}

void Camera::setLookahead(float distance, float speed) {
    _config.lookaheadDistance = distance;
    _config.lookaheadSpeed = speed;
}

void Camera::setDamping(float dampingX, float dampingY) {
    _config.dampingX = dampingX;
    _config.dampingY = dampingY;
}

void Camera::setYStabilization(bool enabled, float threshold) {
    _config.yStabilizationEnabled = enabled;
    _config.yThreshold = threshold;
}

void Camera::setLevelBounds(const sf::FloatRect& bounds) {
    _config.levelBounds = bounds;
    _config.useBounds = true;
    _currentCenter = clampToBounds(_currentCenter);
    _view.setCenter(_currentCenter);
}

void Camera::setMoveSpeed(float speed) {
    _freeMoveSpeed = speed;
}

void Camera::move(float offsetX, float offsetY) {
    _currentCenter += sf::Vector2f(offsetX, offsetY);
    _currentCenter = clampToBounds(_currentCenter);
    _view.setCenter(_currentCenter);
}

void Camera::setCenter(const sf::Vector2f& center) {
    _currentCenter = clampToBounds(center);
    _view.setCenter(_currentCenter);
}

void Camera::setSize(const sf::Vector2f& size) {
    _view.setSize(size);
    _currentCenter = clampToBounds(_currentCenter);
    _view.setCenter(_currentCenter);
}

const sf::View& Camera::getView() const {
    return _view;
}

sf::FloatRect Camera::getViewBounds() const {
    const sf::Vector2f viewSize = _view.getSize();
    return sf::FloatRect(_currentCenter - viewSize * 0.5f, viewSize);
}

void Camera::renderDebug(sf::RenderTarget& target) const {
    // 1. Draw Deadzone Box Outline
    sf::RectangleShape deadzoneRect(_config.deadzoneSize);
    deadzoneRect.setOrigin(_config.deadzoneSize * 0.5f);
    deadzoneRect.setPosition(_currentCenter);
    deadzoneRect.setFillColor(sf::Color(255, 255, 0, 40));
    deadzoneRect.setOutlineColor(sf::Color::Yellow);
    deadzoneRect.setOutlineThickness(2.0f);
    target.draw(deadzoneRect);

    // 2. Draw Y Stabilization Threshold Lines
    if (_config.yStabilizationEnabled) {
        sf::VertexArray yLines(sf::PrimitiveType::Lines);
        const float halfWidth = _view.getSize().x * 0.4f;

        // Top threshold line
        yLines.append(sf::Vertex({_currentCenter.x - halfWidth, _currentCenter.y - _config.yThreshold}, sf::Color::Magenta));
        yLines.append(sf::Vertex({_currentCenter.x + halfWidth, _currentCenter.y - _config.yThreshold}, sf::Color::Magenta));

        // Bottom threshold line
        yLines.append(sf::Vertex({_currentCenter.x - halfWidth, _currentCenter.y + _config.yThreshold}, sf::Color::Magenta));
        yLines.append(sf::Vertex({_currentCenter.x + halfWidth, _currentCenter.y + _config.yThreshold}, sf::Color::Magenta));

        target.draw(yLines);
    }

    // 3. Draw Target Position Marker & Lookahead Offset
    if (_target) {
        const sf::Vector2f targetPos = _target->getPosition();

        sf::CircleShape targetPoint(6.0f);
        targetPoint.setOrigin({6.0f, 6.0f});
        targetPoint.setPosition(targetPos);
        targetPoint.setFillColor(sf::Color::Green);
        target.draw(targetPoint);

        // Line representing lookahead offset vector
        sf::VertexArray lookaheadLine(sf::PrimitiveType::Lines);
        lookaheadLine.append(sf::Vertex(targetPos, sf::Color::Cyan));
        lookaheadLine.append(sf::Vertex(targetPos + sf::Vector2f(_currentLookaheadX, 0.0f), sf::Color::Cyan));
        target.draw(lookaheadLine);
    }

    // 4. Draw Level Bounds Outline (if active)
    if (_config.useBounds) {
        sf::RectangleShape boundsRect(_config.levelBounds.size);
        boundsRect.setPosition(_config.levelBounds.position);
        boundsRect.setFillColor(sf::Color::Transparent);
        boundsRect.setOutlineColor(sf::Color::Cyan);
        boundsRect.setOutlineThickness(3.0f);
        target.draw(boundsRect);
    }
}
