#pragma once

#include <SFML/Graphics.hpp>

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
    void renderDebugGrid(
        sf::RenderTarget& target,
        const WorldMap& worldMap
    ) const;
};
