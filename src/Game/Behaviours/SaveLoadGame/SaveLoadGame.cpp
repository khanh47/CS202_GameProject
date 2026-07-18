#include <iostream>
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>
#include "Game/Behaviours/SaveLoadGame/SaveLoadGame.h"

using namespace std;

using json = nlohmann::json;

string SaveLoadGame::GetFile(int index) {
    if (index >= 0 && index < saveFiles.size()) {
        return saveFiles[index];
    } else {
        cout << "Invalid index!" << endl;
        return "";
    }
}

void SaveLoadGame::SaveGame(const string& saveFileName, const string& gameState) {
    string filenameWithExtension = saveFileName + ".json";
    filesystem::path filePath = saveFilePath / filenameWithExtension;
    
    ofstream outFile(filePath);
    
    json j;

    if (outFile.is_open()) {
        outFile << gameState;
        outFile.close();
        cout << "Game saved to " << filePath << endl;
    } else {
        cout << "Failed to open file for saving!" << endl;
    }
}

void SaveLoadGame::LoadGame(const string& saveFileName, string& GameState) {
    string filenameWithExtension = saveFileName + ".json";
    filesystem::path filePath = saveFilePath / filenameWithExtension;

    ifstream inFile(filePath);

    if (inFile.is_open()) {
        inFile.close();
        cout << "Game Loaded!" << endl;
    }
    else {
        cout << "Failed to open file for loading!" << endl;
    }
}