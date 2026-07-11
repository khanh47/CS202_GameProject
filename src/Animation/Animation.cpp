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
    return animationSet;
}