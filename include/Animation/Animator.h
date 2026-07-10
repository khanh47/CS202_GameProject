#pragma once

#include "Animation/AnimationClip.h"
#include <SFML/Graphics/Rect.hpp>
#include <unordered_map>
#include <string>

class Animator {
public:
    Animator() = default;
    ~Animator() = default;

    void addAnimation(const std::string& name, AnimationClip animation);
    void play(const std::string& name);
    void stop();
    void pause();
    void resume();

    bool update(float deltaTime);

    sf::IntRect getCurrentTextureRect() const;
    bool hasActiveAnimation() const;
    bool isPlaying() const;
    bool isPaused() const;

private:
    std::unordered_map<std::string, AnimationClip> _animations;

    std::string _currentAnimationName;
    std::size_t _currentFrameId = 0;
    float _elapsedTime = 0.f;
    bool _paused = false;
};
