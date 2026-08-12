#include "Game/World/SpawnSpec.h"

#include <stdexcept>

namespace {
ObjectKind parseKind(const nlohmann::json& json) {
    const std::string kind = json.get<std::string>();
    if (kind == "Block") {
        return ObjectKind::Block;
    }
    if (kind == "Player") {
        return ObjectKind::Player;
    }
    if (kind == "Enemy") {
        return ObjectKind::Enemy;
    }
    if (kind == "Item") {
        return ObjectKind::Item;
    }
    if (kind == "Pipe") {
        return ObjectKind::Pipe;
    }
    throw std::runtime_error("Unknown spawn kind: " + kind);
}

sf::Vector2f parseSize(const nlohmann::json& json) {
    if (!json.is_array() || json.size() != 2
        || !json[0].is_number() || !json[1].is_number()) {
        throw std::runtime_error("Spawn size must be a [width, height] pair");
    }
    return {json[0].get<float>(), json[1].get<float>()};
}
}

void from_json(const nlohmann::json& json, SpawnSpec& spec) {
    if (!json.is_object()) {
        throw std::runtime_error("Spawn spec must be a JSON object");
    }
    if (!json.contains("kind")) {
        throw std::runtime_error("Spawn spec is missing required field: kind");
    }
    if (!json.contains("typeKey")) {
        throw std::runtime_error("Spawn spec is missing required field: typeKey");
    }
    if (!json.contains("texture")) {
        throw std::runtime_error("Spawn spec is missing required field: texture");
    }
    if (!json.contains("size")) {
        throw std::runtime_error("Spawn spec is missing required field: size");
    }

    spec.kind = parseKind(json["kind"]);
    spec.typeKey = json["typeKey"].get<std::string>();
    spec.textureKey = json["texture"].get<std::string>();
    spec.size = parseSize(json["size"]);

    spec.animationId = json.value("animationId", "");
    if (json.contains("offset")) {
        spec.offset = parseSize(json["offset"]);
    }
    spec.centerVertically = json.value("centerVertically", false);
    spec.addSeamFilter = json.value("addSeamFilter", false);

    // Pipe-specific fields
    spec.pipeOrientation = json.value("pipeOrientation", "");
    spec.pipeEndSide = json.value("pipeEndSide", "");
    spec.pipeBodyLength = json.value("pipeBodyLength", 1);
    spec.pipeIsWarp = json.value("pipeIsWarp", false);
    spec.warpID = json.value("warpID", -1);
    spec.warpTarget = json.value("warpTarget", -1);
    spec.contentsStatic = json.value("contentsStatic", false);
    spec.addController = json.value("addController", false);

    if (json.contains("contents")) {
        if (json["contents"].is_null()) {
            spec.contents.reset();
        } else {
            spec.contents = std::make_shared<SpawnSpec>(
                json["contents"].get<SpawnSpec>()
            );
        }
    }
}

std::unordered_map<int, std::vector<SpawnSpec>> parseSpawnSpecs(
    const nlohmann::json& json
) {
    std::unordered_map<int, std::vector<SpawnSpec>> result;
    if (!json.is_array()) {
        throw std::runtime_error("Spawn definitions must be an array");
    }

    for (const auto& entry : json) {
        if (!entry.is_object() || !entry.contains("id")
            || !entry["id"].is_number_integer()) {
            throw std::runtime_error(
                "Every spawn definition must have an integer id"
            );
        }
        const int id = entry["id"].get<int>();
        if (id <= 0) {
            throw std::runtime_error("Spawn id must be positive: " + std::to_string(id));
        }

        const SpawnSpec spec = entry.get<SpawnSpec>();
        result[id].push_back(spec);
    }
    return result;
}
