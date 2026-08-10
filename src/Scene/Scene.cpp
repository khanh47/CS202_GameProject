#include "Scene/Scene.h"

#include "ResourceManager.h"

Scene::Scene() = default;

Scene::Scene(const std::string& name)
    : _name(name) {}

void Scene::setBackground(const std::string& textureAlias) {
    const sf::Texture& texture =
        ResourceManager::getInstance().getTexture(textureAlias);
    _backgroundSprite.emplace(texture, sf::IntRect({0, 0}, sf::Vector2i(texture.getSize())));
}

void Scene::renderBackground(sf::RenderTarget& target) {
    if (!_backgroundSprite.has_value()) {
        return;
    }

    const sf::Vector2f viewSize = target.getView().getSize();
    const sf::Vector2f viewPosition =
        target.getView().getCenter() - viewSize * 0.5f;
    const sf::Vector2u textureSize = _backgroundSprite->getTexture().getSize();

    _backgroundSprite->setScale({
        viewSize.x / static_cast<float>(textureSize.x),
        viewSize.y / static_cast<float>(textureSize.y)
    });
    _backgroundSprite->setPosition(viewPosition);
    target.draw(*_backgroundSprite);
}
