#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <vector>

#include "Animation/AnimationFrame.h"

class AnimationClip {
public:
    AnimationClip() = default;
    AnimationClip(std::vector<AnimationFrame> frames, bool looping);
    ~AnimationClip() = default;

    const AnimationFrame& getFrame(int id) const { return frames[id]; }
    const int getFrameCount() { return frames.size(); }
    const bool isEmpty() { return frames.size() == 0; }

private:
    std::vector<AnimationFrame> frames;
    bool looping = true;
};

namespace Animation {
    AnimationClip createClip(sf::IntRect rect, sf::Vector2i offsets, const int& frameCount); // 2 first entries: starting pixel, 2 last: size of a frame   
}