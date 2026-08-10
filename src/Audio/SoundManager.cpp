#include "Audio/SoundManager.h"
#include "ResourceManager.h"
#include "Game/GameSettings.h"
#include <algorithm>

namespace Audio {

SoundManager &SoundManager::getInstance() {
    static SoundManager instance;
    return instance;
}

SoundManager::SoundManager() = default;
SoundManager::~SoundManager() = default;

void SoundManager::preload(const std::string &alias, const std::string &filepath) {
    // Delegate loading to ResourceManager; mapping stored there
    try {
        ResourceManager::getInstance().preLoadSound(filepath, alias);
    } catch (...) {
    }
}

void SoundManager::playEffect(const std::string &alias, float volumeMultiplier) {
    try {
        auto &buf = ResourceManager::getInstance().getSoundBuffer(alias);

        // Find available sound in pool
        for (auto &s : _pool) {
            if (s.getStatus() == sf::SoundSource::Status::Stopped) {
                s.setBuffer(buf);
                s.setVolume(std::clamp(_globalVolume * volumeMultiplier, 0.f, 100.f));
                s.play();
                return;
            }
        }

        if (_pool.size() < _maxPoolSize) {
            // Construct the sound with the buffer (SFML Sound requires a buffer at construction)
            _pool.emplace_back(buf);
            auto &s = _pool.back();
            s.setVolume(std::clamp(_globalVolume * volumeMultiplier, 0.f, 100.f));
            s.play();
        }
    } catch (...) {
    }
}

void SoundManager::setGlobalVolume(float v) {
    _globalVolume = std::clamp(v, 0.f, 100.f);
}

float SoundManager::getGlobalVolume() const { return _globalVolume; }

void SoundManager::stopAll() {
    for (auto &s : _pool) {
        if (s.getStatus() != sf::SoundSource::Status::Stopped) s.stop();
    }
}

void SoundManager::update() {
    // Optionally shrink pool or reuse; currently no-op
}

} // namespace Audio
