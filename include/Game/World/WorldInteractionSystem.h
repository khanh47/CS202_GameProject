#pragma once

#include <box2d/box2d.h>

class FireballPool;
class GameWorld;
class WorldObjectStore;

class WorldInteractionSystem {
public:
    void processContacts(b2ContactEvents events);
    void processSensors(b2SensorEvents events);
    void processObjectInteractions(
        WorldObjectStore& objectStore,
        FireballPool& fireballPool,
        GameWorld& gameWorld
    );
};
