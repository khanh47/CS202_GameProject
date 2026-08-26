#include "Game/Snapshot/SaveLoadGame.h"

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace {
constexpr int saveFormatVersion = 1;

std::string defaultSlotName(int index) {
    return "Slot " + std::to_string(index + 1);
}

std::string currentDateTime() {
    const std::time_t now = std::chrono::system_clock::to_time_t(
        std::chrono::system_clock::now()
    );
    std::tm localTime{};
#ifdef _WIN32
    localtime_s(&localTime, &now);
#else
    localtime_r(&now, &localTime);
#endif

    std::ostringstream output;
    output << std::put_time(&localTime, "%Y-%m-%d %H:%M");
    return output.str();
}
} // namespace

SaveLoadGame& SaveLoadGame::getInstance() {
    static SaveLoadGame instance;
    return instance;
}

SaveLoadGame::SaveLoadGame() {
    std::error_code error;
    std::filesystem::create_directories(saveFilePath, error);

    saveFiles.reserve(SlotCount);
    for (int index = 0; index < SlotCount; ++index) {
        saveFiles.push_back(slotPath(index).string());
    }
}

bool SaveLoadGame::isValidSlot(int index) const noexcept {
    return index >= 0 && index < SlotCount;
}

std::filesystem::path SaveLoadGame::slotPath(int index) const {
    return saveFilePath / ("slot-" + std::to_string(index + 1) + ".json");
}

std::vector<SaveLoadGame::SlotInfo> SaveLoadGame::getSlots() const {
    std::vector<SlotInfo> slots;
    slots.reserve(SlotCount);

    for (int index = 0; index < SlotCount; ++index) {
        SlotInfo slot;
        slot.index = index;

        std::ifstream input(slotPath(index));
        if (input.is_open()) {
            try {
                nlohmann::json root;
                input >> root;
                if (root.is_object()
                    && root.contains("state")
                    && root["state"].is_object()) {
                    slot.exists = true;
                    slot.name = root.value("slotName", defaultSlotName(index));
                    slot.savedDate = root.value("savedAt", "");
                }
            } catch (const nlohmann::json::exception&) {
                slot.exists = false;
            }
        }

        slots.push_back(std::move(slot));
    }

    return slots;
}

bool SaveLoadGame::hasAnySave() const {
    for (const SlotInfo& slot : getSlots()) {
        if (slot.exists) {
            return true;
        }
    }
    return false;
}

bool SaveLoadGame::saveSlot(
    int index,
    const std::string& slotName,
    const nlohmann::json& gameState
) {
    if (!isValidSlot(index) || !gameState.is_object()) {
        return false;
    }

    std::error_code error;
    std::filesystem::create_directories(saveFilePath, error);
    if (error) {
        return false;
    }

    nlohmann::json root;
    root["formatVersion"] = saveFormatVersion;
    root["slotName"] = slotName.empty()
        ? defaultSlotName(index)
        : slotName;
    root["savedAt"] = currentDateTime();
    root["state"] = gameState;

    std::ofstream output(slotPath(index), std::ios::trunc);
    if (!output.is_open()) {
        return false;
    }

    output << root.dump(4);
    return output.good();
}

bool SaveLoadGame::loadSlot(
    int index,
    SlotInfo& slot,
    nlohmann::json& gameState
) const {
    slot = SlotInfo{};
    slot.index = index;
    gameState = nlohmann::json{};
    if (!isValidSlot(index)) {
        return false;
    }

    std::ifstream input(slotPath(index));
    if (!input.is_open()) {
        return false;
    }

    try {
        nlohmann::json root;
        input >> root;
        if (!root.is_object()
            || !root.contains("state")
            || !root["state"].is_object()) {
            return false;
        }

        slot.exists = true;
        slot.name = root.value("slotName", defaultSlotName(index));
        slot.savedDate = root.value("savedAt", "");
        gameState = root["state"];
        return true;
    } catch (const nlohmann::json::exception&) {
        return false;
    }
}

void SaveLoadGame::setCurrentSession(
    const nlohmann::json& gameState,
    bool hasUnsavedChanges
) {
    if (!gameState.is_object()) {
        clearCurrentSession();
        return;
    }
    _currentSession = gameState;
    _sessionDirty = hasUnsavedChanges;
}

void SaveLoadGame::clearCurrentSession() {
    _currentSession.reset();
    _sessionDirty = false;
}

std::string SaveLoadGame::GetFile(int index) const {
    return (index >= 0 && index < static_cast<int>(saveFiles.size()))
        ? saveFiles[static_cast<std::size_t>(index)]
        : std::string{};
}

void SaveLoadGame::SaveGame(
    const std::string& saveFileName,
    const std::string& gameState
) {
    std::error_code error;
    std::filesystem::create_directories(saveFilePath, error);
    if (error) {
        return;
    }

    std::ofstream output(saveFilePath / (saveFileName + ".json"));
    if (output.is_open()) {
        output << gameState;
    }
}

void SaveLoadGame::LoadGame(
    const std::string& saveFileName,
    std::string& gameState
) const {
    gameState.clear();
    std::ifstream input(saveFilePath / (saveFileName + ".json"));
    if (!input.is_open()) {
        return;
    }

    std::ostringstream contents;
    contents << input.rdbuf();
    gameState = contents.str();
}
