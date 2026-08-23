#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "Scene/Scene.h"
#include "Game/World/GameWorld.h"
#include "Game/Camera.h"
#include "Game/ScoreManager.h"



class InGameScene : public Scene {
public:
    explicit InGameScene(const std::string& name);
    ~InGameScene() override = default;

    void init() override;
    void onEnter() override;
    void onExit() override;
    void handleInput(const sf::Event& event) override;
    void updateSimulation(const float &fixedDt) override;
    void updateVisuals(float deltaTime) override;
    void render(sf::RenderTarget& target) override;

private:
    void _checkGameOver();
    void _checkWin();
    void _checkMinigameResult();
    void _respawnPlayer();
    void _drawGameOverOverlay(sf::RenderTarget& target);
    void _drawWinOverlay(sf::RenderTarget& target);

    bool _winReactionActive = false;
    bool _gameOverActive = false;
    bool _winActive = false;
    bool _starmanMusicActive = false;
    std::size_t _minigameParticipantCount = 0;
    std::shared_ptr<Player> _minigameWinner;
    std::optional<sf::Sprite> _gameOverOverlay;
    std::optional<sf::Text> _gameOverPrompt;
    const sf::Texture* _gameOverTexture = nullptr;
    std::optional<sf::Text> _winTitle;
    std::optional<sf::Text> _winPrompt;

    GameWorld _gameWorld;
    Camera _camera;
    ScoreManager _scoreManager;
};
