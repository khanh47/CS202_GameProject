#include "Game/Objects/Player/State/NormalState.h"

#include <utility>

NormalState::NormalState(std::string character)
    : _character(std::move(character)) {}
