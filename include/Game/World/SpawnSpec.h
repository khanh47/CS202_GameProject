#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <SFML/System/Vector2.hpp>
#include <nlohmann/json.hpp>

#include "Game/Minigame/MinigameTypes.h"

enum class ObjectKind {
    Block,
    Player,
    Enemy,
    Item,
    Pipe
};

struct SpawnSpec {
    ObjectKind kind = ObjectKind::Item;
    std::string typeKey;
    std::string animationId;
    std::string textureKey;
    sf::Vector2f size;
    sf::Vector2f offset;
    bool centerVertically = false;
    bool addSeamFilter = false; // = true for Block so that Player wont flickering 
    bool addController = false; // add controller for player
    PlayerSlot playerSlot = PlayerSlot::Unassigned;
    std::shared_ptr<SpawnSpec> contents; // to contains other objects inside of it

    // Pipe-specific fields
    std::string pipeOrientation; // "vertical" or "horizontal"
    std::string pipeEndSide;     // "top", "bottom", "left", "right"
    int pipeBodyLength = 1;      // Number of repeating body segments
    bool pipeIsWarp = false;
    int warpID = -1;
    int warpTarget = -1;
    bool contentsStatic = false; // Keep Pipe contents stationary
};

void from_json(const nlohmann::json& json, SpawnSpec& spec);
std::unordered_map<int, std::vector<SpawnSpec>> parseSpawnSpecs(
    const nlohmann::json& json
);
