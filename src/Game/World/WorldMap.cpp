#include "Game/World/WorldMap.h"

#include <algorithm>
#include <memory>

#include "Game/Objects/GameObject.h"
#include "Game/Objects/GameObjectFactory.h"
#include "Game/Objects/Item/FireballPool.h"
#include "Game/Objects/Item/Coin.h"
#include "Game/Objects/Player/Player.h"
#include "Game/UserInput/PlayerController.h"
#include "Game/World/GameWorld.h"
#include "Game/World/WorldObjectStore.h"
#include "Physics/PhysicsWorld.h"
#include "ResourceManager.h"

WorldMap::WorldMap(int gridWidth, int gridHeight, float cellSize)
    : _cellSize(cellSize),
      _gridWidth(gridWidth),
      _gridHeight(gridHeight),
      _grid(
          gridHeight,
          std::vector<std::weak_ptr<GameObject>>(gridWidth)
      ) {}

void WorldMap::rebuild(
    const std::vector<std::vector<int>>& mapData,
    PhysicsWorld& physicsWorld,
    GameObjectFactory& objectFactory,
    FireballPool& fireballPool,
    WorldObjectStore& objectStore,
    GameWorld& gameWorld
) {
    auto& resources = ResourceManager::getInstance();
    sf::Texture& brickTexture = resources.getTexture("brick");
    sf::Texture& marioTexture = resources.getTexture("mario_spritesheet");
    sf::Texture& goombaTexture = resources.getTexture("goomba_spritesheet");
    sf::Texture& koopaTexture = resources.getTexture("koopa_spritesheet");
    sf::Texture& itemsTexture = resources.getTexture("mario_and_items");
    sf::Texture& coinsTexture = resources.getTexture("coin_spritesheet");

    objectStore.clear();
    fireballPool.initialize(physicsWorld, itemsTexture);
    _terrainSeamFilter.clear();
    _terrainSeamFilter.install(physicsWorld);

    _loadedRows = std::min(static_cast<int>(mapData.size()), _gridHeight);
    _loadedColumns = 0;
    for (int row = 0; row < _loadedRows; ++row) {
        _loadedColumns = std::max(
            _loadedColumns,
            std::min(static_cast<int>(mapData[row].size()), _gridWidth)
        );
    }

    _tileMap.initialize(_gridWidth, _gridHeight, _cellSize);
    _tileMap.setTexture(&brickTexture);
    _grid.assign(
        _gridHeight,
        std::vector<std::weak_ptr<GameObject>>(_gridWidth)
    );

    for (int mapRow = 0; mapRow < _loadedRows; ++mapRow) {
        const int columns = std::min(
            static_cast<int>(mapData[mapRow].size()),
            _gridWidth
        );
        const int logicY = logicYForMapRow(mapRow);
        const int screenY = screenYForMapRow(mapRow);

        for (int column = 0; column < columns; ++column) {
            const int tileId = mapData[mapRow][column];
            if (tileId == 0) {
                continue;
            }
            const sf::Vector2f spawnPosition = {
                column * _cellSize + _cellSize * 0.5f,
                screenY * _cellSize + _cellSize * 0.5f
            };

            if (tileId == 1) {
                _tileMap.setTile(column, screenY, tileId);
                auto block = objectFactory.createBlock(
                    "Block",
                    &brickTexture
                );
                block->spawn(
                    physicsWorld,
                    spawnPosition,
                    {_cellSize, _cellSize}
                );
                if (logicY >= 0 && logicY < _gridHeight) {
                    _grid[logicY][column] = block;
                }
                _terrainSeamFilter.addBlock(
                    block,
                    column,
                    screenY,
                    column * _cellSize,
                    (column + 1) * _cellSize
                );
                objectStore.addObject(std::move(block));
            } else if (tileId == 2) {
                auto player = objectFactory.createPlayer(
                    "Player",
                    &marioTexture,
                    "mario"
                );
                player->spawn(
                    physicsWorld,
                    {spawnPosition.x + 10.0f, spawnPosition.y},
                    {72.0f, 120.0f},
                    true
                );
                if (auto mario = std::dynamic_pointer_cast<Player>(player)) {
                    mario->changeToNormalState();
                    objectStore.addController(
                        std::make_unique<PlayerController>(
                            *mario,
                            gameWorld,
                            PlayerController::ControlScheme::Wasd
                        )
                    );
                }
                objectStore.addObject(std::move(player));
            } else if (tileId == 4) {
                auto goomba = objectFactory.createEnemy(
                    "Goomba",
                    &goombaTexture,
                    "goomba"
                );
                goomba->spawn(
                    physicsWorld,
                    spawnPosition,
                    {60.0f, 72.0f}
                );
                objectStore.addObject(std::move(goomba));
            } else if (tileId == 5) {
                auto koopa = objectFactory.createEnemy(
                    "Koopa",
                    &koopaTexture,
                    "koopa"
                );
                koopa->spawn(
                    physicsWorld,
                    spawnPosition,
                    {64.0f, 100.0f}
                );
                objectStore.addObject(std::move(koopa));
            } else if (tileId == 6) {
                auto fireFlower = objectFactory.createItem(
                    "FireFlower",
                    &itemsTexture
                );
                constexpr sf::Vector2f flowerSize{54.0f, 54.0f};
                const sf::Vector2f flowerPosition = {
                    spawnPosition.x,
                    spawnPosition.y + (_cellSize - flowerSize.y) * 0.5f
                };
                fireFlower->spawn(
                    physicsWorld,
                    flowerPosition,
                    flowerSize
                );
                objectStore.addObject(std::move(fireFlower));
            }
            else if (tileId == 7) {
                auto coin = objectFactory.createItem(
                    "Coin", 
                    &coinsTexture
                );
                constexpr sf::Vector2f coinSize{54.0f, 54.0f};
                const sf::Vector2f coinPosition = {
                    spawnPosition.x,
                    spawnPosition.y + (_cellSize - coinSize.y) * 0.5f
                };
                coin->spawn(
                    physicsWorld,
                    coinPosition,
                    coinSize
                );
                objectStore.addObject(std::move(coin));
            }
        }
    }
}

void WorldMap::renderTiles(sf::RenderTarget& target) {
    _tileMap.updateVisibleVertices(target.getView());
    target.draw(_tileMap);
}

sf::FloatRect WorldMap::getBounds() const {
    const float width = (_loadedColumns > 0 ? _loadedColumns : _gridWidth)
        * _cellSize;
    return {{0.0f, 0.0f}, {width, _gridHeight * _cellSize}};
}

sf::Vector2f WorldMap::mapCellCenter(int column, int mapRow) const {
    return {
        column * _cellSize + _cellSize * 0.5f,
        screenYForMapRow(mapRow) * _cellSize + _cellSize * 0.5f
    };
}

int WorldMap::logicYForMapRow(int mapRow) const noexcept {
    return _loadedRows - mapRow;
}

int WorldMap::screenYForMapRow(int mapRow) const noexcept {
    return screenRowFor(mapRow, _loadedRows, _gridHeight);
}
