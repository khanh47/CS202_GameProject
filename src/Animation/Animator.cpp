#include "Animation/Animator.h"
#include "Animation/AnimationClip.h"
#include <SFML/Graphics/Rect.hpp>
#include <string>
#include <iostream>

void Animator::addAnimation(const std::string& name, AnimationClip animation) {
    _animations[name] = std::move(animation);
}

void Animator::play(const std::string& name) {
    if(_animations.find(name) == _animations.end()) return;

    _currentAnimationName = name;
    _currentFrameId = 0;
}

void Animator::step(int cnt) {
    if(_currentAnimationName.empty()) return;

    AnimationClip& animation = _animations.at(_currentAnimationName);
    if(animation.isEmpty()) return;
    _currentFrameId = (_currentFrameId + cnt) % animation.getFrameCount();
}

sf::IntRect Animator::getCurrentTextureRect() const {
    return _animations.at(_currentAnimationName).getFrame(_currentFrameId).rect;
}

float Animator::getCurrentFrameDuration() const {
return _animations.at(_currentAnimationName).getFrame(_currentFrameId).duration;
}

bool Animator::hasActiveAnimation() const {
    return !_currentAnimationName.empty() && _animations.find(_currentAnimationName) != _animations.end();
}
