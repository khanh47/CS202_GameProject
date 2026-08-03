#include "Game/Objects/Player/State/SuperState.h"

#include <utility>

SuperState::SuperState(std::string character)
    : _character(std::move(character)) {}
