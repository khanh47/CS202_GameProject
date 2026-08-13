#pragma once

inline bool isPlayerStompContact(
    int contactPointCount,
    float normalFromPlayerY
) noexcept {
    return contactPointCount > 0 && normalFromPlayerY >= 0.5f;
}
