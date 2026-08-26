#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

class SaveLoadGame {
public:
    static constexpr int SlotCount = 3;

    struct SlotInfo {
        int index = -1;
        bool exists = false;
        std::string name;
        std::string savedDate;
    };

    static SaveLoadGame& getInstance();

    SaveLoadGame();
    virtual ~SaveLoadGame() = default;

    std::vector<SlotInfo> getSlots() const;
    bool hasAnySave() const;
    bool saveSlot(
        int index,
        const std::string& slotName,
        const nlohmann::json& gameState
    );
    bool loadSlot(
        int index,
        SlotInfo& slot,
        nlohmann::json& gameState
    ) const;

    void setCurrentSession(
        const nlohmann::json& gameState,
        bool hasUnsavedChanges = true
    );
    void clearCurrentSession();
    bool hasCurrentSession() const noexcept {
        return _currentSession.has_value();
    }
    const nlohmann::json* getCurrentSession() const noexcept {
        return _currentSession ? &*_currentSession : nullptr;
    }
    bool hasUnsavedSession() const noexcept {
        return _currentSession.has_value() && _sessionDirty;
    }
    void markSessionSaved() noexcept { _sessionDirty = false; }

    // Compatibility helpers for the original string-based save prototype.
    std::string GetFile(int index) const;
    virtual void SaveGame(
        const std::string& saveFileName,
        const std::string& gameState
    );
    virtual void LoadGame(
        const std::string& saveFileName,
        std::string& gameState
    ) const;

    std::string operator[](int index) const {
        return (index >= 0 && index < static_cast<int>(saveFiles.size()))
            ? saveFiles[static_cast<std::size_t>(index)]
            : std::string{};
    }

private:
    bool isValidSlot(int index) const noexcept;
    std::filesystem::path slotPath(int index) const;

    std::vector<std::string> saveFiles;
    std::filesystem::path saveFilePath = "assets/SaveGameFiles";
    std::optional<nlohmann::json> _currentSession;
    bool _sessionDirty = false;
};
