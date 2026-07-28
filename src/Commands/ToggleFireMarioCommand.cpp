#include "Commands/ToggleFireMarioCommand.h"
#include "Game/GameSettings.h"

void ToggleFireMarioCommand::execute() {
    GameSettings& settings = GameSettings::getInstance();
    settings.useFireMario = !settings.useFireMario;
}
