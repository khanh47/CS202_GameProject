#pragma once

#include "Animation/AnimationClip.h"
#include <SFML/Graphics/Rect.hpp>
#include <string>

class Animator {
public:
    Animator() = default;
    ~Animator() = default;

    void addAnimation(const std::string& name, AnimationClip animation);
    void play(const std::string& name);

    void step(int cnt = 1);

    sf::IntRect getCurrentTextureRect() const;
    float getCurrentFrameDuration() const;
    bool hasActiveAnimation() const;

private:
    std::unordered_map<std::string, AnimationClip> _animations;

    std::string _currentAnimationName;
    int _currentFrameId = 0;
};
