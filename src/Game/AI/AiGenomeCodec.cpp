#include "Game/AI/AiGenomeCodec.h"

#include <cmath>
#include <fstream>
#include <stdexcept>
#include <system_error>

#include <nlohmann/json.hpp>

#ifdef _WIN32
#include <windows.h>
#endif

namespace {
void validateNetwork(const nlohmann::json& document) {
    if (document.value("version", 0) != AiPolicy::modelVersion) {
        throw std::runtime_error("Unsupported AI model version");
    }
    if (!document.contains("network") || !document["network"].is_object()) {
        throw std::runtime_error("AI model is missing its network description");
    }
    const auto& network = document["network"];
    if (network.value("inputs", 0) != static_cast<int>(AiPolicy::inputCount)
        || network.value("hidden", 0) != static_cast<int>(AiPolicy::hiddenCount)
        || network.value("outputs", 0) != static_cast<int>(AiPolicy::outputCount)) {
        throw std::runtime_error("AI model network dimensions are incompatible");
    }
}

void replaceFile(const std::filesystem::path& temporaryPath,
                 const std::filesystem::path& destinationPath) {
#ifdef _WIN32
    if (!MoveFileExW(
            temporaryPath.c_str(),
            destinationPath.c_str(),
            MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        throw std::runtime_error("Unable to replace AI model file");
    }
#else
    std::filesystem::rename(temporaryPath, destinationPath);
#endif
}
}

AiGenome AiGenomeCodec::load(const std::filesystem::path& path) {
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("Unable to open AI model: " + path.string());
    }

    nlohmann::json document;
    try {
        input >> document;
    } catch (const nlohmann::json::exception& error) {
        throw std::runtime_error("Malformed AI model JSON: " + std::string(error.what()));
    }
    if (!document.is_object()) {
        throw std::runtime_error("AI model must be a JSON object");
    }
    validateNetwork(document);
    if (!document.contains("weights") || !document["weights"].is_array()) {
        throw std::runtime_error("AI model is missing its weights array");
    }

    AiGenome genome;
    try {
        genome.weights = document["weights"].get<std::vector<float>>();
    } catch (const nlohmann::json::exception& error) {
        throw std::runtime_error("Invalid AI model weights: " + std::string(error.what()));
    }
    if (genome.weights.size() != AiPolicy::weightCount) {
        throw std::runtime_error(
            "AI model has " + std::to_string(genome.weights.size())
            + " weights; expected " + std::to_string(AiPolicy::weightCount)
        );
    }
    for (const float weight : genome.weights) {
        if (!std::isfinite(weight)) {
            throw std::runtime_error("AI model contains a non-finite weight");
        }
    }

    if (document.contains("training") && document["training"].is_object()) {
        const auto& training = document["training"];
        genome.trainingMap = training.value("map", "");
        genome.trainingSeed = training.value("seed", 0U);
        genome.generation = training.value("generation", 0);
        genome.fitness = training.value("fitness", 0.0f);
    }
    if (!std::isfinite(genome.fitness)) {
        throw std::runtime_error("AI model contains non-finite training metadata");
    }
    return genome;
}

void AiGenomeCodec::save(
    const std::filesystem::path& path,
    const AiGenome& genome
) {
    AiPolicy validated(genome);
    (void)validated;

    if (!path.parent_path().empty()) {
        std::filesystem::create_directories(path.parent_path());
    }
    const std::filesystem::path temporaryPath = path.string() + ".tmp";
    const nlohmann::json document = {
        {"version", AiPolicy::modelVersion},
        {"network", {
            {"inputs", AiPolicy::inputCount},
            {"hidden", AiPolicy::hiddenCount},
            {"outputs", AiPolicy::outputCount}
        }},
        {"weights", genome.weights},
        {"training", {
            {"map", genome.trainingMap},
            {"seed", genome.trainingSeed},
            {"generation", genome.generation},
            {"fitness", genome.fitness}
        }}
    };

    {
        std::ofstream output(temporaryPath, std::ios::trunc);
        if (!output) {
            throw std::runtime_error("Unable to write temporary AI model file");
        }
        output << document.dump(2) << '\n';
        output.flush();
        if (!output) {
            throw std::runtime_error("Failed while writing AI model file");
        }
    }

    try {
        replaceFile(temporaryPath, path);
    } catch (...) {
        std::error_code ignored;
        std::filesystem::remove(temporaryPath, ignored);
        throw;
    }
}
