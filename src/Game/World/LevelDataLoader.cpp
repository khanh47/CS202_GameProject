#include "Game/World/LevelDataLoader.h"

#include <fstream>
#include <stdexcept>
#include <string>

#include <nlohmann/json.hpp>

std::vector<std::vector<int>> LevelDataLoader::load(
    const std::filesystem::path& filePath,
    int maximumWidth,
    int maximumHeight
) {
    if (maximumWidth <= 0 || maximumHeight <= 0) {
        throw std::invalid_argument("Level limits must be positive");
    }

    std::ifstream input(filePath);
    if (!input) {
        throw std::runtime_error("Unable to open level file: " + filePath.string());
    }

    nlohmann::json document;
    try {
        input >> document;
    } catch (const nlohmann::json::exception& error) {
        throw std::runtime_error("Malformed level JSON: " + std::string(error.what()));
    }

    if (!document.is_object() || !document.contains("rows")
        || !document["rows"].is_array()) {
        throw std::runtime_error("Level JSON must contain a rows array");
    }

    const auto& encodedRows = document["rows"];
    if (encodedRows.empty() || encodedRows.size() > static_cast<std::size_t>(maximumHeight)) {
        throw std::runtime_error("Level height is outside configured limits");
    }

    std::vector<std::vector<int>> rows;
    rows.reserve(encodedRows.size());
    for (const auto& encodedRow : encodedRows) {
        if (!encodedRow.is_string()) {
            throw std::runtime_error("Every level row must be a digit string");
        }
        const std::string rowText = encodedRow.get<std::string>();
        if (rowText.empty() || rowText.size() > static_cast<std::size_t>(maximumWidth)) {
            throw std::runtime_error("Level row width is outside configured limits");
        }

        std::vector<int> row;
        row.reserve(rowText.size());
        for (const char tile : rowText) {
            if (tile < '0' || tile > '7') {
                throw std::runtime_error("Level contains an unsupported tile id");
            }
            row.push_back(tile - '0');
        }
        rows.push_back(std::move(row));
    }
    return rows;
}
