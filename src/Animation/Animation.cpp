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

AnimationSet Animation::makeGoombaAnimationSet() {
    AnimationSet animationSet;
    animationSet.defaultClip = "walk";

    animationSet.clips.emplace(
        "walk",
        Animation::createLinearClip(
            {2, 42},       // startPosition: Top-left pixel of the very first Goomba frame
            {16, 19},      // frameSize: Width and Height of the bounding b
            4,            // frameCount: 12 frames total (3x4 grid)
            {17, 0},      // frameStride: Pixel distance from top-left of Frame 1 to top-left of Frame 2 (X and Y)
            1.0f / 4.0f,  // frameDuration: 12 frames per second
            true           // looping
        )
    );

    animationSet.clips.emplace(
        "dead",
        Animation::createLinearClip(
            {4, 450},      // startPosition: Placeholder for the top-left pixel of the squished Goomba
            {32, 16},      // frameSize: Bounding box (notice the height is smaller since it's flat!)
            1,             // frameCount: Only 1 frame
            {0, 0},        // frameStride: 0, since there are no other frames to move to
            1.0f,          // frameDuration: Doesn't matter much for 1 frame, but 1.0f is safe
            false          // looping: FALSE! We want it to stay dead, not restart.
        )
    );

    return animationSet;
}