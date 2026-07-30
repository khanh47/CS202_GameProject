#include <filesystem>
#include <vector>

#pragma once

using namespace std;

class SaveLoadGame {
private:
    vector <string> saveFiles;
    filesystem::path saveFilePath = "assets/SaveGameFiles";
public:
    SaveLoadGame();
    virtual ~SaveLoadGame() = default;

    string GetFile(int index);
    virtual void SaveGame(const string& saveFileName, const string& gameState);
    virtual void LoadGame(const string& saveFileName, string& gameState);
    string operator [](int index) {
        return saveFiles[index];
    }
};