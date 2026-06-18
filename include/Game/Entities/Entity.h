#pragma once

#include "box2d/box2d.h"

class Entity {
public:
    virtual ~Entity() = default;

    virtual void update(float dt) {};
    virtual void render() {};
    
    void destroy(){
        pendingDestroy = true;
    }
    
    bool isPendingDestroy(){
        return pendingDestroy;
    }

protected:

    bool pendingDestroy = false;
};