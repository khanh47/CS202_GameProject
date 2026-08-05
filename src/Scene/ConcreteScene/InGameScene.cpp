#include <memory>

#include "Scene/ConcreteScene/InGameScene.h"
#include "ResourceManager.h"
#include "Scene/SceneManager.h"
#include "Game/GameSettings.h"

InGameScene::InGameScene(const std::string& name)
    : _name(name) {
}

void InGameScene::init() {
    _gameOverActive = false;
    _gameOverTexture = &ResourceManager::getInstance().getTexture("game_over");
    _gameOverOverlay.emplace(*_gameOverTexture);
    _gameOverPrompt.emplace(
        ResourceManager::getInstance().getFont("Roboto"),
        "Press any key to continue",
        67
    );
    _gameOverPrompt->setFillColor(sf::Color::White);
    _gameWorld.loadLevel(_name);
    _gameWorld.setScoreManager(&_scoreManager); // Set score manager for the game world

    // Configure 2D Platformer Camera System parameters
    CameraConfig config;
    config.deadzoneSize = {250.0f, 180.0f};          // Invisible rectangular deadzone box
    config.lookaheadDistance = 160.0f;               // Forward anticipation distance
    config.lookaheadSpeed = 3.5f;                    // Interpolation speed for lookahead transitions
    config.dampingX = 6.0f;                          // Horizontal smooth damping factor
    config.dampingY = 4.5f;                          // Vertical smooth damping factor
    config.yStabilizationEnabled = true;             // Ignore minor vertical hops/jumps
    config.yThreshold = 140.0f;                      // Vertical displacement threshold for Y tracking
    config.levelBounds = _gameWorld.getBounds();     // Clamp view within level limits
    config.useBounds = true;

    _camera.setConfig(config);

    // Bind camera tracking target to player
    if (auto player = _gameWorld.getPrimaryPlayer()) {
        _camera.setTarget(player);
    } else {
        _camera.setCenter({1920.f / 2.f, _gameWorld.getGridHeight() * _gameWorld.getCellSize() - 1080.f / 2.f});
    }
}

void InGameScene::onEnter() {
    _isActive = true;
}

void InGameScene::onExit() {
    _isActive = false;
}

void InGameScene::cleanup() {
}

void InGameScene::handleInput(const sf::Event& event) {
    if (_gameOverActive) {
        if (event.is<sf::Event::KeyPressed>()
            || event.is<sf::Event::MouseButtonPressed>()
            || event.is<sf::Event::JoystickButtonPressed>()) {
            if (auto mgr = getSceneManager()) {
                mgr->requestPopScene();
            }
        }
        return;
    }

    if (auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
        if (keyEvent->code == sf::Keyboard::Key::Escape) {
            if (auto mgr = getSceneManager()) {
                mgr->requestPopScene();
                return;
            }
        }
    }

    _gameWorld.handleInput(event);
}

void InGameScene::updateSimulation(const float &fixedDt) {
    if (_gameOverActive) {
        return;
    }

    _gameWorld.updateSimulation(fixedDt);
    _checkGameOver();
}

void InGameScene::updateVisuals(float deltaTime) {
    if (_gameOverActive) {
        return;
    }

    _gameWorld.updateVisuals(deltaTime);
    _camera.update(deltaTime);
    if (!_camera.getTarget() && _gameWorld.getPrimaryPlayer()) {
        _camera.setTarget(_gameWorld.getPrimaryPlayer());
    }
    _scoreManager.update(deltaTime);
}

void InGameScene::render(sf::RenderTarget& target) {
    sf::View defaultView = target.getDefaultView();
    target.setView(_camera.getView());

    _gameWorld.render(target);

    // Render floating score popups in world coordinates
    const sf::Font& font = ResourceManager::getInstance().getFont("Roboto");
    _scoreManager.renderFloatingTexts(target, font);

    // Render camera debug overlays (deadzone, lookahead line, level bounds) when debug grid is enabled
    if (GameSettings::getInstance().debugDrawGrid) {
        _camera.renderDebug(target);
    }

    target.setView(defaultView);

    // Render screen HUD overlay
    _scoreManager.renderHUD(target, font, sf::Vector2f(40.f, 30.f));

    if (_gameOverActive) {
        _drawGameOverOverlay(target);
    }
}

void InGameScene::_checkGameOver() {
    if (_gameOverActive) {
        return;
    }

    if (_gameWorld.hasLivingPlayers()) {
        return;
    }

    _gameOverActive = true;
    if (_gameOverOverlay.has_value() && _gameOverTexture) {
        _gameOverOverlay->setTexture(*_gameOverTexture);
    }
}

void InGameScene::_drawGameOverOverlay(sf::RenderTarget& target) {
    if (!_gameOverOverlay.has_value() || !_gameOverTexture || !_gameOverPrompt.has_value()) {
        return;
    }

    const sf::View view = target.getDefaultView();
    target.setView(view);

    const sf::Vector2u textureSize = _gameOverTexture->getSize();
    const sf::Vector2f viewSize = view.getSize();
    if (textureSize.x == 0 || textureSize.y == 0) {
        return;
    }

    _gameOverOverlay->setOrigin({
        static_cast<float>(textureSize.x) * 0.5f,
        static_cast<float>(textureSize.y) * 0.5f
    });
    _gameOverOverlay->setPosition({viewSize.x * 0.5f, viewSize.y * 0.5f});
    _gameOverOverlay->setScale({
        viewSize.x / static_cast<float>(textureSize.x),
        viewSize.y / static_cast<float>(textureSize.y)
    });
    _gameOverOverlay->setColor(sf::Color(255, 255, 255, 210));

    target.draw(*_gameOverOverlay);

    _gameOverPrompt->setOrigin(_gameOverPrompt->getLocalBounds().position + (_gameOverPrompt->getLocalBounds().size * 0.5f));
    _gameOverPrompt->setPosition({viewSize.x * 0.5f, viewSize.y * 0.93f});
    _gameOverPrompt->setFillColor(sf::Color(255, 255, 255, 235));
    target.draw(*_gameOverPrompt);
}
