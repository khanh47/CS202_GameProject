#pragma once

#include "Animation/AnimationClip.h"
#include "Animation/AnimationSet.h"

namespace Animation {
    AnimationClip createLinearClip(
        sf::Vector2i startPosition,
        sf::Vector2i frameSize,
        int frameCount,
        sf::Vector2i frameStride,
        float frameDuration,
        bool looping = true
    );

    AnimationSet makeDefaultPlayerAnimationSet();
    AnimationSet makeGoombaAnimationSet();
    AnimationSet makeKoopaAnimationSet();
    AnimationSet makeFireFlowerAnimationSet();
    AnimationSet makeFireballAnimationSet();
    AnimationSet makeFireTransformAnimationSet();
    AnimationSet makeCoinAnimationSet();
}
