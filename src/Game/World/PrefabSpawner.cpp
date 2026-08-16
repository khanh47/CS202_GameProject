#include "Game/World/PrefabSpawner.h"

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>

#include "Game/GameSettings.h"
#include "Game/Objects/Block/Block.h"
#include "Game/Objects/Block/SlopeBlock.h"
#include "Game/Objects/Enemy/ConcreteEnemy/Koopa.h"
#include "Game/Objects/Enemy/ConcreteEnemy/PiranhaPlant.h"
#include "Game/Objects/Enemy/Enemy.h"
#include "Game/Objects/GameObject.h"
#include "Game/Objects/GameObjectFactory.h"
#include "Game/Objects/Pipe/Pipe.h"
#include "Game/Objects/Player/Player.h"
#include "Game/UserInput/PlayerController.h"
#include "Game/World/GameWorld.h"
#include "Game/World/TerrainSeamFilter.h"
#include "Game/World/WorldObjectStore.h"
#include "Physics/PhysicsWorld.h"
#include "ResourceManager.h"

namespace {
bool isSelectedCharacter(const SpawnSpec& spec) {
    const GameSettings& settings = GameSettings::getInstance();
    if (settings.gameMode != GameMode::Solo) {
        return true;
    }

    const bool isLuigi = spec.animationId.find("luigi") != std::string::npos;
    return (isLuigi ? "luigi" : "mario") == settings.player1Character;
}

sf::Vector2f gridAlignedPosition(
    const SpawnSpec& spec,
    float cellSize,
    const sf::Vector2f& cellCenter
) {
    const float verticalOffset = spec.centerVertically
        ? (cellSize - spec.size.y) * 0.5f
        : 0.0f;
    return {
        cellCenter.x + spec.offset.x,
        cellCenter.y + spec.offset.y + verticalOffset
    };
}
}

PrefabSpawner::PrefabSpawner(
    PhysicsWorld& physicsWorld,
    GameObjectFactory& objectFactory,
    WorldObjectStore& objectStore,
    GameWorld& gameWorld,
    TerrainSeamFilter& terrainSeamFilter,
    float cellSize
)
    : _physicsWorld(physicsWorld),
      _objectFactory(objectFactory),
      _objectStore(objectStore),
      _gameWorld(gameWorld),
      _terrainSeamFilter(terrainSeamFilter),
      _cellSize(cellSize) {}

std::shared_ptr<GameObject> PrefabSpawner::spawnAtGrid(
    const SpawnSpec& spec,
    int column,
    int screenRow,
    const sf::Vector2f& cellCenter
) {
    if (!spec.objectKind) {
        return nullptr;
    }

    const sf::Vector2f position = gridAlignedPosition(
        spec,
        _cellSize,
        cellCenter
    );
    if (*spec.objectKind == ObjectKind::Pipe) {
        return spawnPipe(spec, column, screenRow, cellCenter, position);
    }
    return spawnObjectAtPosition(spec, position, column, screenRow);
}

std::shared_ptr<GameObject> PrefabSpawner::spawnAtPosition(
    const SpawnSpec& spec,
    const sf::Vector2f& position
) {
    if (!spec.objectKind || *spec.objectKind == ObjectKind::Pipe) {
        throw std::runtime_error(
            "spawnAtPosition only supports dynamic object prefabs"
        );
    }
    return spawnObjectAtPosition(spec, position, -1, -1);
}

std::shared_ptr<GameObject> PrefabSpawner::spawnObjectAtPosition(
    const SpawnSpec& spec,
    const sf::Vector2f& position,
    int column,
    int screenRow
) {
    if (!spec.objectKind) {
        return nullptr;
    }

    if (*spec.objectKind == ObjectKind::Player && !isSelectedCharacter(spec)) {
        return nullptr;
    }

    sf::Texture* texture = nullptr;
    if (!spec.textureKey.empty()) {
        texture = &ResourceManager::getInstance().getTexture(
            spec.textureKey
        );
    }

    std::shared_ptr<GameObject> object;
    switch (*spec.objectKind) {
        case ObjectKind::Block: {
            auto block = _objectFactory.createBlock(spec.typeKey, texture);
            if (auto typedBlock = std::dynamic_pointer_cast<Block>(block)) {
                typedBlock->setBreakable(spec.breakable);
            }
            if (!spec.slopeType.empty() && texture != nullptr) {
                if (auto slope = std::dynamic_pointer_cast<SlopeBlock>(block)) {
                    slope->configureSlopeVisuals(
                        *texture,
                        SlopeBlock::parseSlopeType(spec.slopeType)
                    );
                }
            }
            block->spawn(_physicsWorld, position, spec.size);
            if (spec.addSeamFilter && column >= 0 && screenRow >= 0) {
                _terrainSeamFilter.addBlock(
                    block,
                    column,
                    screenRow,
                    column * _cellSize,
                    (column + 1) * _cellSize
                );
            }
            object = std::move(block);
            break;
        }
        case ObjectKind::Player: {
            auto player = _objectFactory.createPlayer(
                spec.typeKey,
                texture,
                spec.animationId
            );
            player->spawn(_physicsWorld, position, spec.size);

            if (auto mario = std::dynamic_pointer_cast<Player>(player)) {
                mario->changeToNormalState();
                mario->setGameWorld(_gameWorld);
                if (spec.addController) {
                    const GameSettings& settings = GameSettings::getInstance();
                    const bool useWasd = settings.gameMode == GameMode::Solo
                        || spec.animationId == "mario";
                    _objectStore.addController(
                        std::make_unique<PlayerController>(
                            *mario,
                            _gameWorld,
                            useWasd
                                ? PlayerController::ControlScheme::Wasd
                                : PlayerController::ControlScheme::ArrowKeys
                        )
                    );
                }
            }
            object = std::move(player);
            break;
        }
        case ObjectKind::Enemy: {
            auto enemy = _objectFactory.createEnemy(
                spec.typeKey,
                texture,
                spec.animationId
            );
            enemy->spawn(_physicsWorld, position, spec.size);
            if (auto typedEnemy = std::dynamic_pointer_cast<Enemy>(enemy)) {
                typedEnemy->setSupportGrid(
                    &_terrainSeamFilter,
                    _cellSize
                );
            }
            if (auto koopa = std::dynamic_pointer_cast<Koopa>(enemy)) {
                koopa->setGameWorld(&_gameWorld);
            }
            object = std::move(enemy);
            break;
        }
        case ObjectKind::Item: {
            auto item = _objectFactory.createItem(spec.typeKey, texture);
            item->spawn(_physicsWorld, position, spec.size);
            object = std::move(item);
            break;
        }
        case ObjectKind::Pipe:
            return nullptr;
    }

    _objectStore.addObject(object);
    return object;
}

std::shared_ptr<GameObject> PrefabSpawner::spawnPipe(
    const SpawnSpec& spec,
    int column,
    int screenRow,
    const sf::Vector2f& cellCenter,
    const sf::Vector2f& spawnPosition
) {
    sf::Texture& texture = ResourceManager::getInstance().getTexture(
        spec.textureKey
    );
    auto pipe = _objectFactory.createPipe(
        spec.typeKey,
        &texture,
        spec.pipeOrientation,
        spec.pipeEndSide,
        spec.pipeBodyLength,
        spec.pipeIsWarp,
        spec.warpID,
        spec.warpTarget
    );

    const Pipe::Orientation orientation =
        spec.pipeOrientation == "horizontal"
        ? Pipe::Orientation::Horizontal
        : Pipe::Orientation::Vertical;
    const sf::Vector2f pipeSize = Pipe::computePipeSize(
        orientation,
        spec.pipeBodyLength,
        _cellSize
    );

    // A pipe entry identifies the cap cell. Its body is then positioned from
    // the cap direction, so its physics footprint and visual footprint agree.
    sf::Vector2f pipePosition = spawnPosition;
    const float halfCell = _cellSize * 0.5f;
    if (spec.pipeOrientation == "vertical") {
        pipePosition.x += halfCell;
        if (spec.pipeEndSide == "bottom") {
            pipePosition.y = cellCenter.y - halfCell + pipeSize.y * 0.5f;
        } else {
            pipePosition.y = cellCenter.y + halfCell - pipeSize.y * 0.5f;
        }
    } else {
        pipePosition.y += halfCell;
        if (spec.pipeEndSide == "right") {
            pipePosition.x = cellCenter.x - halfCell + pipeSize.x * 0.5f;
        } else {
            pipePosition.x = cellCenter.x + halfCell - pipeSize.x * 0.5f;
        }
    }

    pipe->spawn(_physicsWorld, pipePosition, pipeSize);

    if (spec.contents) {
        const SpawnSpec& content = *spec.contents;
        sf::Vector2f contentPosition = pipePosition + content.offset;
        float hiddenY = contentPosition.y;
        float emergedY = contentPosition.y;
        if (spec.pipeOrientation == "vertical"
            && spec.pipeEndSide == "top") {
            const float pipeTopY = pipePosition.y - pipeSize.y * 0.5f;
            constexpr float hiddenDepthPixels = 8.0f;
            hiddenY = pipeTopY + content.size.y * 0.5f
                + hiddenDepthPixels;
            emergedY = pipeTopY - content.size.y * 0.5f;
            contentPosition.y = spec.contentsStatic
                ? emergedY
                : hiddenY + content.offset.y;
        }

        const std::shared_ptr<GameObject> contentObject =
            spawnObjectAtPosition(content, contentPosition, -1, -1);
        if (!spec.contentsStatic) {
            if (auto plant = std::dynamic_pointer_cast<PiranhaPlant>(
                    contentObject
                )) {
                plant->setPipeTravel(
                    hiddenY + content.offset.y,
                    emergedY
                );
            }
        }
    }

    if (spec.addSeamFilter && column >= 0 && screenRow >= 0) {
        _terrainSeamFilter.addBlock(
            pipe,
            column,
            screenRow,
            column * _cellSize,
            (column + 1) * _cellSize
        );

        const int pipeTiles = 1 + std::max(spec.pipeBodyLength, 0);
        if (spec.pipeOrientation == "vertical") {
            const int firstRow = spec.pipeEndSide == "bottom"
                ? screenRow
                : screenRow - pipeTiles + 1;
            for (int row = firstRow; row < firstRow + pipeTiles; ++row) {
                _terrainSeamFilter.addOccupiedCell(pipe, column, row);
                _terrainSeamFilter.addOccupiedCell(pipe, column + 1, row);
            }
        } else {
            const int firstColumn = spec.pipeEndSide == "right"
                ? column
                : column - pipeTiles + 1;
            for (int pipeColumn = firstColumn;
                 pipeColumn < firstColumn + pipeTiles;
                 ++pipeColumn) {
                _terrainSeamFilter.addOccupiedCell(
                    pipe,
                    pipeColumn,
                    screenRow
                );
                _terrainSeamFilter.addOccupiedCell(
                    pipe,
                    pipeColumn,
                    screenRow + 1
                );
            }
        }
    }

    _objectStore.addObject(pipe);
    return pipe;
}
