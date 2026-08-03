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
    preloadEnemyAnimationSets();
    preloadCoinAnimationSets();
}

void AnimationLibrary::registerAnimationSet(const std::string& name, AnimationSet animationSet) {
    _animationSets.emplace(name, std::move(animationSet));
}

void AnimationLibrary::preloadPlayerAnimationSets() {
    const AnimationSet playerSet = Animation::makeDefaultPlayerAnimationSet();
    registerAnimationSet("mario", playerSet);
    registerAnimationSet("fire_mario", playerSet);
    registerAnimationSet("fire_luigi", playerSet);
    registerAnimationSet("luigi", playerSet);

    const AnimationSet fireTransformSet = Animation::makeFireTransformAnimationSet();
    registerAnimationSet("transform_fire", fireTransformSet);
}

void AnimationLibrary::preloadEnemyAnimationSets() {
    const AnimationSet goombaSet = Animation::makeGoombaAnimationSet();
    registerAnimationSet("goomba", goombaSet);

    const AnimationSet koopaSet = Animation::makeKoopaAnimationSet();
    registerAnimationSet("koopa", koopaSet);

    const AnimationSet fireFlowerSet = Animation::makeFireFlowerAnimationSet();
    registerAnimationSet("fire_flower", fireFlowerSet);

    const AnimationSet fireballSet = Animation::makeFireballAnimationSet();
    registerAnimationSet("fireball", fireballSet);
}

void AnimationLibrary::preloadCoinAnimationSets() {
    const AnimationSet coinSet = Animation::makeCoinAnimationSet();
    registerAnimationSet("coin", coinSet);
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
