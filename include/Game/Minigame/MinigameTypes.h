#pragma once

enum class PlayerSlot {
    Unassigned = 0,
    One = 1,
    Two = 2
};

enum class MinigameResult {
    Running,
    PlayerOneWon,
    PlayerTwoWon,
    Draw,
    Timeout
};
