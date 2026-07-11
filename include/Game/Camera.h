#pragma once

#include <SFML/Graphics.hpp>
#include <memory>

class GameObject;

class Camera {
public:
    Camera();
    Camera(const sf::Vector2f& size);

    void update(float deltaTime);

    void setTarget(std::shared_ptr<GameObject> target);
    void move(float offsetX, float offsetY);
    void setCenter(const sf::Vector2f& center);

    const sf::View& getView() const;
    void setSize(const sf::Vector2f& size);

private:
    sf::View _view;
    std::shared_ptr<GameObject> _target;
    float _moveSpeed;
};
