#include "Commands/ToggleDebugCommands.h"
#include "Game/GameSettings.h"

void ToggleGridCommand::execute() {
    auto& settings = GameSettings::getInstance();
    settings.debugDrawGrid = !settings.debugDrawGrid;
}

void ToggleCoordinatesCommand::execute() {
    auto& settings = GameSettings::getInstance();
    settings.debugDrawCoordinates = !settings.debugDrawCoordinates;
}

void ToggleFreeCameraCommand::execute() {
    auto& settings = GameSettings::getInstance();
    settings.freeCameraMove = !settings.freeCameraMove;
}
