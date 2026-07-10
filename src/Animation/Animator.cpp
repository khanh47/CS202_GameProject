#include "Animation/Animator.h"
#include "Animation/AnimationClip.h"
#include <SFML/Graphics/Rect.hpp>
#include <cstddef>
#include <string>
#include <utility>

void Animator::addAnimation(const std::string& name, AnimationClip animation) {
    _animations[name] = std::move(animation);
}

void Animator::play(const std::string& name) {
    if (_animations.find(name) == _animations.end()) return;

    _currentAnimationName = name;
    _currentFrameId = 0;
    _elapsedTime = 0.f;
    _paused = false;
}

void Animator::stop() {
    _currentAnimationName.clear();
    _currentFrameId = 0;
    _elapsedTime = 0.f;
    _paused = false;
}

void Animator::pause() {
    if (hasActiveAnimation()) {
        _paused = true;
    }
}

void Animator::resume() {
    if (hasActiveAnimation()) {
        _paused = false;
    }
}

bool Animator::update(float deltaTime) {
    if (!isPlaying() || deltaTime <= 0.f) {
        return false;
    }

    AnimationClip& animation = _animations.at(_currentAnimationName);
    if (animation.isEmpty()) {
        return false;
    }

    _elapsedTime += deltaTime;
    bool frameChanged = false;

    while (true) {
        const AnimationFrame& currentFrame = animation.getFrame(_currentFrameId);
        if (currentFrame.duration <= 0.f || _elapsedTime < currentFrame.duration) {
            break;
        }

        _elapsedTime -= currentFrame.duration;

        if (_currentFrameId + 1 < animation.getFrameCount()) {
            ++_currentFrameId;
            frameChanged = true;
            continue;
        }

        if (animation.isLooping()) {
            _currentFrameId = 0;
            frameChanged = true;
            continue;
        }

        _currentFrameId = animation.getFrameCount() - 1;
        _paused = true;
        _elapsedTime = 0.f;
        frameChanged = true;
        break;
    }

    return frameChanged;
}

sf::IntRect Animator::getCurrentTextureRect() const {
    if (!hasActiveAnimation()) {
        return {};
    }

    const AnimationClip& animation = _animations.at(_currentAnimationName);
    if (animation.isEmpty()) {
        return {};
    }

    return animation.getFrame(_currentFrameId).rect;
}

bool Animator::hasActiveAnimation() const {
    return !_currentAnimationName.empty() && _animations.find(_currentAnimationName) != _animations.end();
}

bool Animator::isPlaying() const {
    return hasActiveAnimation() && !_paused;
}

bool Animator::isPaused() const {
    return hasActiveAnimation() && _paused;
}
