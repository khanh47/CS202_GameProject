#pragma once

#include <filesystem>
#include <vector>

class LevelDataLoader {
public:
    static std::vector<std::vector<int>> load(
        const std::filesystem::path& filePath,
        int maximumWidth = 500,
        int maximumHeight = 60
    );
};
