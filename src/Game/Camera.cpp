#include "Game/Camera.h"
#include "Game/Objects/GameObject.h"
#include "Game/GameSettings.h"

Camera::Camera() : _moveSpeed(2000.0f) {
    _view.setSize({1920.f, 1080.f});
    _view.setCenter({1920.f / 2.f, 1080.f / 2.f});
}

Camera::Camera(const sf::Vector2f& size) : _moveSpeed(500.0f) {
    _view.setSize(size);
    _view.setCenter({size.x / 2.f, size.y / 2.f});
}

void Camera::update(float deltaTime) {
    if (_target) {
        // Follow the target if one is set
        setCenter(_target->getPosition());
    } else if (GameSettings::getInstance().freeCameraMove) {
        // Free movement with keyboard if no target and free movement is enabled
        float offsetX = 0.0f;
        float offsetY = 0.0f;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
            offsetX -= _moveSpeed * deltaTime;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
            offsetX += _moveSpeed * deltaTime;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
            offsetY -= _moveSpeed * deltaTime;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
            offsetY += _moveSpeed * deltaTime;
        }

        move(offsetX, offsetY);
    }
}

void Camera::setTarget(std::shared_ptr<GameObject> target) {
    _target = target;
}

void Camera::move(float offsetX, float offsetY) {
    _view.move({offsetX, offsetY});
}

void Camera::setCenter(const sf::Vector2f& center) {
    _view.setCenter(center);
}

const sf::View& Camera::getView() const {
    return _view;
}

void Camera::setSize(const sf::Vector2f& size) {
    _view.setSize(size);
}
