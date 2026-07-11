#include "Game/GameSettings.h"

GameSettings& GameSettings::getInstance() {
    static GameSettings instance;
    return instance;
}
