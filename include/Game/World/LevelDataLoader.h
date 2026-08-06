#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include "Game/World/SpawnSpec.h"

struct LevelData {
    std::vector<std::vector<int>> rows;
    std::unordered_map<int, std::vector<SpawnSpec>> spawns;
    std::string background;
};

class LevelDataLoader {
public:
    static LevelData load(
        const std::filesystem::path& filePath,
        int maximumWidth = 500,
        int maximumHeight = 60,
        const std::filesystem::path& sharedSpawnsPath =
            "assets/datas/spawns.json"
    );
};
