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
            true
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
            1.0f / 3.0f,
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
            {2, 42},
            {16, 19},
            4,       
            {17, 0}, 
            1.0f / 4.0f,
            true        
        )
    );

    animationSet.clips.emplace(
        "dead",
        Animation::createLinearClip(
            {4, 450},   
            {32, 16},   
            1,          
            {0, 0},     
            1.0f,       
            false       
        )
    );

    return animationSet;
}

AnimationSet Animation::makeKoopaAnimationSet() {
    AnimationSet animationSet;
    animationSet.defaultClip = "walk";

animationSet.clips.emplace(
        "walk",
        Animation::createLinearClip(
            {6, 32},
            {24, 32},
            8,        
            {28, 0},
            1.0f / 8.0f,
            true         
        )
    );

    animationSet.clips.emplace(
        "dead",
        Animation::createLinearClip(
            {180, 160},
            {20, 16},
            1,          
            {0, 0},      
            1.0f,
            false 
        )
    );

    animationSet.clips.emplace(
        "slide",
        Animation::createLinearClip(
            {216, 269},
            {16, 14},
            4,          
            {19, 0},
            1.0f / 4.0f,        
            true 
        )
    );

    return animationSet;
}

AnimationSet Animation::makeFireFlowerAnimationSet() {
    AnimationSet animationSet;
    animationSet.defaultClip = "idle";

    animationSet.clips.emplace(
        "idle",
        Animation::createLinearClip(
            {0, 109},
            {18, 17},
            4,
            {18, 0},
            1.0f / 6.0f,
            true
        )
    );

    return animationSet;
}

AnimationSet Animation::makeFireballAnimationSet() {
    AnimationSet animationSet;
    animationSet.defaultClip = "spin";

    const float frameDuration = 1.0f / 8.0f;
    std::vector<AnimationFrame> frames = {
        AnimationFrame(sf::IntRect({4, 148}, {8, 10}), frameDuration),
        AnimationFrame(sf::IntRect({23, 148}, {8, 10}), frameDuration),
        AnimationFrame(sf::IntRect({42, 148}, {8, 10}), frameDuration),
        AnimationFrame(sf::IntRect({59, 148}, {8, 10}), frameDuration)
    };

    animationSet.clips.emplace("spin", AnimationClip(frames, true));

    return animationSet;
}

AnimationSet Animation::makeTransformAnimationSet() {
    AnimationSet animationSet;
    animationSet.defaultClip = "transform";

    const float dt = 0.08f;
    // Rapidly alternate between transformation frames (Normal -> Intermediate -> Fire)
    std::vector<AnimationFrame> frames = {
        AnimationFrame(sf::IntRect({0, 0}, {32, 32}), dt),
        AnimationFrame(sf::IntRect({32, 0}, {32, 32}), dt),
        AnimationFrame(sf::IntRect({64, 0}, {32, 32}), dt),
        AnimationFrame(sf::IntRect({32, 0}, {32, 32}), dt),
        AnimationFrame(sf::IntRect({0, 0}, {32, 32}), dt),
        AnimationFrame(sf::IntRect({32, 0}, {32, 32}), dt),
        AnimationFrame(sf::IntRect({64, 0}, {32, 32}), dt),
        AnimationFrame(sf::IntRect({32, 0}, {32, 32}), dt),
        AnimationFrame(sf::IntRect({0, 0}, {32, 32}), dt),
        AnimationFrame(sf::IntRect({32, 0}, {32, 32}), dt),
        AnimationFrame(sf::IntRect({64, 0}, {32, 32}), dt)
    };

    animationSet.clips.emplace("transform", AnimationClip(frames, false));
    return animationSet;
}

AnimationSet Animation::makeSuperMushroomAnimationSet() {
    AnimationSet animationSet;
    animationSet.defaultClip = "idle";

    animationSet.clips.emplace(
        "idle",
        Animation::createLinearClip(
            {0, 90},
            {18, 18},
            6,
            {18, 0},
            1.0f / 6.0f,
            true
        )
    );

    return animationSet;
}

AnimationSet Animation::makeSuperStarAnimationSet() {
    AnimationSet animationSet;
    animationSet.defaultClip = "idle";

    animationSet.clips.emplace(
        "idle",
        Animation::createLinearClip(
            {0, 126},
            {18, 18},
            4,
            {18, 0},
            1.0f / 6.0f,
            true
        )
    );

    return animationSet;
}

AnimationSet Animation::makeCoinAnimationSet() {
    AnimationSet set;
    set.defaultClip = "idle";
    
    set.clips.emplace(
        "idle", 
        Animation::createLinearClip (
            {0, 0},
            {64, 64},
            4, 
            {72, 0},
            1.0f / 6.0f,
            true
        )
    );

    return set;
}

AnimationSet Animation::makeCoinBlockAnimationSet() {
    AnimationSet set;
    set.defaultClip = "shining";
    
    set.clips.emplace(
        "shining", 
        Animation::createLinearClip (
            {0, 0},
            {64, 64},
            4, 
            {72, 0},
            1.0f / 4.0f,
            true
        )
    );

    set.clips.emplace(
        "final",
        Animation::createLinearClip(
            {288, 0},
            {64, 64},
            1,
            {0, 0},
            1.0f,
            false
        )
    );

    return set;
}