#pragma once

#include <filesystem>

#include "Game/AI/AiPolicy.h"

class AiGenomeCodec {
public:
    static AiGenome load(const std::filesystem::path& path);
    static void save(const std::filesystem::path& path, const AiGenome& genome);
};
