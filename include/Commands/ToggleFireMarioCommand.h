#pragma once

#include "Commands/ICommand.h"

/**
 * @brief Command to toggle Fire Mario setting in GameSettings.
 */
class ToggleFireMarioCommand : public ICommand {
public:
    void execute() override;
    std::string getName() const override { return "ToggleFireMarioCommand"; }
    CommandType getType() const override { return CommandType::IMMEDIATE; }
};
