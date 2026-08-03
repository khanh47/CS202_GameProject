#pragma once

#include <SFML/Graphics.hpp>
#include <string>

class FireballPool;
class WorldMap;
class WorldObjectStore;

class WorldRenderer {
public:
    void render(
        sf::RenderTarget& target,
        WorldMap& worldMap,
        const WorldObjectStore& objectStore,
        FireballPool& fireballPool
    ) const;

private:
    void renderBackground(
        sf::RenderTarget& target,
        const std::string& backgroundKey
    ) const;
    void renderDebugGrid(
        sf::RenderTarget& target,
        const WorldMap& worldMap
    ) const;
};
