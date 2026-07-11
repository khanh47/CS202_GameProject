#pragma once

#include "Animation/AnimationSet.h"

#include <SFML/Graphics/Rect.hpp>
#include <memory>
#include <string>

class Animator {
public:
    Animator();
    Animator(std::shared_ptr<AnimationSet> animationSet);
    ~Animator() = default;

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
    std::shared_ptr<AnimationSet> _animations;

    std::string _currentAnimationName;
    std::size_t _currentFrameId = 0;
    float _elapsedTime = 0.f;
    bool _paused = false;
};
