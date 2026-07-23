#include "Animation/Animation.h"

AnimationClip Animation::createLinearClip(
    sf::Vector2i startPosition,
    sf::Vector2i frameSize,
    int frameCount,
    sf::Vector2i frameStride,
    float frameDuration,
    bool looping
) {
    std::vector<AnimationFrame> frames;
    frames.reserve(frameCount);

    for (int i = 0; i < frameCount; i++) {
        sf::IntRect pos = sf::IntRect(
            {startPosition.x + i * frameStride.x, startPosition.y + i * frameStride.y},
            {frameSize.x, frameSize.y}
        );

        frames.push_back(AnimationFrame(pos, frameDuration));
    }

    return AnimationClip(frames, looping);
}


AnimationSet Animation::makeDefaultPlayerAnimationSet() {
    AnimationSet animationSet;
    animationSet.defaultClip = "idle";

    animationSet.clips.emplace(
        "idle",
        Animation::createLinearClip(
            {48, 32},
            {32, 32},
            4,
            {32, 0},
            1.0f / 4.0f,
            true
        )
    );

    animationSet.clips.emplace(
        "walk",
        Animation::createLinearClip(
            {48, 80},
            {32, 32},
            6,
            {32, 0},
            1.0f / 6.0f,
            true
        )
    );

    animationSet.clips.emplace(
        "jump",
        Animation::createLinearClip(
            {48, 128},
            {32, 32},
            3,
            {32, 0},
            1.0f / 3.0f,
            false
        )
    );

    animationSet.clips.emplace(
        "bump",
        Animation::createLinearClip(
            {176, 128},
            {32, 32},
            2,
            {32, 0},
            1.0f / 4.0f,
            false
        )
    );

    animationSet.clips.emplace(
        "shoot",
        Animation::createLinearClip(
            {288, 352},
            {32, 32},
            3,
            {32, 0},
            1.0f / 4.0f,
            false
        )
    );

    animationSet.clips.emplace(
        "air_shot",
        Animation::createLinearClip(
            {288, 400},
            {32, 32},
            3,
            {32, 0},
            1.0f / 4.0f,
            false
        )
    );

    animationSet.clips.emplace(
        "hit",
        Animation::createLinearClip(
            {288, 352},
            {32, 32},
            3,
            {32, 0},
            1.0f / 4.0f,
            false
        )
    );

    animationSet.clips.emplace(
        "knockout",
        Animation::createLinearClip(
            {48, 512},
            {32, 32},
            9,
            {32, 0},
            1.0f / 7.0f,
            false
        )
    );

    return animationSet;
}