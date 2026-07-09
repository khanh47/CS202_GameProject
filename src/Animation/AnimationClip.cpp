#include "Animation/AnimationClip.h"
#include "Animation/AnimationFrame.h"
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <vector>

AnimationClip::AnimationClip(std::vector<AnimationFrame> frames,bool looping):
frames(std::move(frames)), looping(looping) {

}

AnimationClip Animation::createClip(sf::IntRect rect, sf::Vector2i offsets, const int& frameCount) {
    std::vector<AnimationFrame> frames;
    for(int i = 0; i < frameCount; i++){
        sf::IntRect pos = sf::IntRect(
            {rect.position.x + i * rect.size.x + offsets.x, rect.position.y + offsets.y},
            {rect.size.x - offsets.x, rect.size.y - offsets.y}
        );

        frames.push_back(AnimationFrame(pos,  100.0f));
    }

    return AnimationClip(frames, true);
}