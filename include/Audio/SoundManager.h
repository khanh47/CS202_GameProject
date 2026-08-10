#pragma once

#include <SFML/Audio.hpp>
#include <string>
#include <vector>
#include <memory>

namespace Audio {

class SoundManager {
public:
    static SoundManager &getInstance();

    // Preload a sound buffer alias via ResourceManager (delegated)
    void preload(const std::string &alias, const std::string &filepath);

    // Play an effect by alias. volumeMultiplier multiplies global SFX volume.
    void playEffect(const std::string &alias, float volumeMultiplier = 1.0f);

    // Global SFX volume (0-100)
    void setGlobalVolume(float v);
    float getGlobalVolume() const;

    void stopAll();

    // Call periodically to reclaim finished sounds (optional)
    void update();

private:
    SoundManager();
    ~SoundManager();

    std::vector<sf::Sound> _pool;
    float _globalVolume = 80.f;
    size_t _maxPoolSize = 32;
};

} // namespace Audio
