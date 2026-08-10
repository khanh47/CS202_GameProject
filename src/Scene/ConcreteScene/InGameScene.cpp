#include <memory>

#include "Scene/ConcreteScene/InGameScene.h"
#include "Game/Behaviours/Animatable.h"
#include "Game/Objects/Player/Player.h"
#include "ResourceManager.h"
#include "Audio/MusicManager.h"
#include "Scene/SceneManager.h"
#include "Game/GameSettings.h"
#include <iostream>

InGameScene::InGameScene(const std::string& name)
    : Scene(name) {}

void InGameScene::init() {
    _winReactionActive = false;
    _gameOverActive = false;
    _winActive = false;
    _gameOverTexture = &ResourceManager::getInstance().getTexture("game_over");
    _gameOverOverlay.emplace(*_gameOverTexture);
    _gameOverPrompt.emplace(
        ResourceManager::getInstance().getFont("SuperMario"),
        "Press any key to continue",
        67
    );
    _gameOverPrompt->setFillColor(sf::Color::White);
    _winTitle.emplace(
        ResourceManager::getInstance().getFont("SuperMario"),
        "COURSE CLEAR!",
        82
    );
    _winTitle->setFillColor(sf::Color(255, 227, 102));
    _winPrompt.emplace(
        ResourceManager::getInstance().getFont("SuperMario"),
        "Press any key to continue",
        67
    );
    _winPrompt->setFillColor(sf::Color::White);
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
        // Ensure title-screen music is stopped and play level theme
        _isActive = true;
        stopTitleScreenMusic();
        Audio::MusicManager::getInstance().setVolume(GameSettings::getInstance().musicVolume);

        std::string theme = "ground_theme";
        if (_name.find("map-2") != std::string::npos || _name.find("map-3") != std::string::npos) {
            theme = "underground_theme";
        }

        Audio::MusicManager::getInstance().play(theme, true);
    }

    void InGameScene::onExit() {
        // Stop any level music when leaving the scene
        Audio::MusicManager::getInstance().stop();
        Scene::onExit();
    }

void InGameScene::handleInput(const sf::Event& event) {
    if (_winReactionActive) {
        return;
    }

    if (_winActive) {
        if (event.is<sf::Event::KeyPressed>()
            || event.is<sf::Event::MouseButtonPressed>()
            || event.is<sf::Event::JoystickButtonPressed>()) {
            if (auto mgr = getSceneManager()) {
                mgr->requestPopScene();
            }
        }
        return;
    }

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
    if (_winReactionActive || _gameOverActive || _winActive) {
        return;
    }

    _gameWorld.updateSimulation(fixedDt);
    _checkWin();
    _checkGameOver();
}

void InGameScene::updateVisuals(float deltaTime) {
    _gameWorld.updateVisuals(deltaTime);

    if (_winReactionActive) {
        auto player = std::dynamic_pointer_cast<Player>(_gameWorld.getPrimaryPlayer());
        auto* animatable = player ? player->getBehaviour<Animatable>() : nullptr;
        if (!animatable || animatable->isAnimationDone()) {
            _winReactionActive = false;
            _winActive = true;
        }
    }

    if (!_winReactionActive && !_gameOverActive && !_winActive) {
        _camera.update(deltaTime);
        if (!_camera.getTarget() && _gameWorld.getPrimaryPlayer()) {
            _camera.setTarget(_gameWorld.getPrimaryPlayer());
        }
    }

    _scoreManager.update(deltaTime);
}

void InGameScene::render(sf::RenderTarget& target) {
    sf::View defaultView = target.getDefaultView();
    target.setView(_camera.getView());

    _gameWorld.render(target);

    // Render floating score popups in world coordinates
    const sf::Font& font = ResourceManager::getInstance().getFont("SuperMario");
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
    } else if (_winActive) {
        _drawWinOverlay(target);
    }
}

void InGameScene::_checkWin() {
    if (_winReactionActive || _winActive || _gameOverActive) {
        return;
    }

    if (!_gameWorld.hasWon()) {
        return;
    }

    _winReactionActive = true;

    auto player = std::dynamic_pointer_cast<Player>(_gameWorld.getPrimaryPlayer());
    if (player) {
        if (auto* animatable = player->getBehaviour<Animatable>()) {
            animatable->playAnimation("victory");
        }
    }
}

void InGameScene::_checkGameOver() {
    if (_winReactionActive || _gameOverActive || _winActive) {
        return;
    }

    if (_gameWorld.hasLivingPlayers()) {
        return;
    }

    // Mario died! Deduct 1 life from ScoreManager
    int remainingLives = _scoreManager.getLives() - 1;
    _scoreManager.setLives(remainingLives);
    if (remainingLives > 0) {
        // Player still has lives left -> Respawn Mario / reload level
        _respawnPlayer();
    } else {
        // 0 lives left -> Trigger Game Over screen
        _gameOverActive = true;
        if (_gameOverOverlay.has_value() && _gameOverTexture) {
            _gameOverOverlay->setTexture(*_gameOverTexture);
        }
    }
}

void InGameScene::_respawnPlayer() {
    _gameWorld.respawnPlayer();
    _gameWorld.setScoreManager(&_scoreManager);
    // Rebind camera tracking to the newly spawned player
    if (auto player = _gameWorld.getPrimaryPlayer()) {
        _camera.setTarget(player);
    }
}

void InGameScene::_drawWinOverlay(sf::RenderTarget& target) {
    const sf::View view = target.getDefaultView();
    target.setView(view);

    sf::RectangleShape backdrop(view.getSize());
    backdrop.setFillColor(sf::Color(0, 0, 0, 190));
    backdrop.setPosition({0.0f, 0.0f});
    target.draw(backdrop);

    if (_winTitle.has_value()) {
        _winTitle->setOrigin(_winTitle->getLocalBounds().position + (_winTitle->getLocalBounds().size * 0.5f));
        _winTitle->setPosition({view.getSize().x * 0.5f, view.getSize().y * 0.42f});
        target.draw(*_winTitle);
    }

    if (_winPrompt.has_value()) {
        _winPrompt->setOrigin(_winPrompt->getLocalBounds().position + (_winPrompt->getLocalBounds().size * 0.5f));
        _winPrompt->setPosition({view.getSize().x * 0.5f, view.getSize().y * 0.58f});
        _winPrompt->setFillColor(sf::Color(255, 255, 255, 235));
        target.draw(*_winPrompt);
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
