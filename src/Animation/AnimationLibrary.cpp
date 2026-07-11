#include "Animation/AnimationLibrary.h"
#include "Animation/AnimationSet.h"
#include "Animation/Animation.h"

#include <stdexcept>
#include <utility>

AnimationLibrary& AnimationLibrary::getInstance() {
    static AnimationLibrary instance;
    return instance;
}

AnimationLibrary::AnimationLibrary() {
    preloadPlayerAnimationSets();
}

void AnimationLibrary::registerAnimationSet(const std::string& name, AnimationSet animationSet) {
    _animationSets.emplace(name, std::move(animationSet));
}

void AnimationLibrary::preloadPlayerAnimationSets() {
    const AnimationSet playerSet = Animation::makeDefaultPlayerAnimationSet();
    registerAnimationSet("mario", playerSet);
    registerAnimationSet("luigi", playerSet);
}

const AnimationSet& AnimationLibrary::getAnimationSet(const std::string& name) const {
    const auto it = _animationSets.find(name);
    if (it == _animationSets.end()) {
        throw std::runtime_error("Unknown animation set: " + name);
    }

    return it->second;
}

const AnimationSet& AnimationLibrary::getPlayerAnimationSet(const std::string& name) const {
    return getAnimationSet(name);
}
