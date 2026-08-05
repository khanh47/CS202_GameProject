#include "Game/Objects/Player/State/FireState.h"

#include <utility>

FireState::FireState(std::string character)
    : _character(std::move(character)) {}
