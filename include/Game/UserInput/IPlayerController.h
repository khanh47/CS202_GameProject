#pragma once

#include <SFML/Window/Event.hpp>

class IPlayerController {
public:
    virtual ~IPlayerController() = default;

    virtual bool handleEvent(const sf::Event& event) {
        (void)event;
        return false;
    }
    virtual void fixedUpdate(float fixedDt) { (void)fixedDt; }
    virtual void syncState() {}
    virtual bool isPlayerPendingDestroy() const = 0;
};
