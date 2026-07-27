# Development Guide

## Creating a New Game Object with Animation (e.g. Goomba)

This guide walks through adding a new animated game object to the project.

---

### Step 1 — Create the Class Files

Create header and source under `include/Game/Objects/Enemy/` and `src/Game/Objects/Enemy/`:

**`include/Game/Objects/Enemy/Goomba.h`**

```cpp
#pragma once

#include <box2d/box2d.h>
#include "Game/Behaviours/Animatable.h"
#include "Game/Behaviours/Damageable.h"
#include "Game/Behaviours/Moveable.h"
#include "Game/Objects/GameObject.h"
#include "Physics/PhysicsWorld.h"

class Goomba : public GameObject,
               public Animatable,
               public Damageable,
               public Moveable {
public:
    Goomba();
    Goomba(sf::Texture& texture);
    ~Goomba();

protected:
    void onCreateBodyDef(b2BodyDef& def) override;
    void onCreateShapeDef(b2ShapeDef& def) override;
    void onUpdateVisuals(float deltaTime) override;
    void onRenderVisual(sf::RenderTarget& target,
                        const sf::Vector2f& position,
                        float angleDegrees) override;
    void updateSimulation(const float& fixedDt) override;
};
```

**`src/Game/Objects/Enemy/Goomba.cpp`**

```cpp
#include "Game/Objects/Enemy/Goomba.h"
#include "Physics/PhysicsUnits.h"

Goomba::Goomba()
    : GameObject(), Animatable(), Damageable(20) {
}

Goomba::Goomba(sf::Texture& texture)
    : GameObject(), Animatable(), Damageable(20) {
    configureVisuals(texture, "goomba");
}

Goomba::~Goomba() = default;

void Goomba::onCreateBodyDef(b2BodyDef& def) {
    def.type = b2_dynamicBody;
    startMoveLeft();
}

void Goomba::onCreateShapeDef(b2ShapeDef& def) {
    def.density = 1.0f;
    def.friction = 0.3f;
}

void Goomba::updateSimulation(const float& fixedDt) {
    if (!hasValidBody()) return;

    b2BodyId body = _body->getId();
    float speed = 3.0f;
    float dir = isFacingLeft() ? -1.0f : 1.0f;
    b2Vec2 vel = b2Body_GetLinearVelocity(body);
    b2Body_SetLinearVelocity(body, { speed * dir, vel.y });
}

void Goomba::onUpdateVisuals(float deltaTime) {
    updateVisualState(deltaTime, _hitboxPixels, isFacingLeft());
}

void Goomba::onRenderVisual(sf::RenderTarget& target,
                            const sf::Vector2f& position,
                            float angleDegrees) {
    renderVisualState(target, position, angleDegrees);
}
```

**Key points:**
- Call `configureVisuals(texture, "goomba")` in the constructor to bind the spritesheet and load the animation set.
- `onUpdateVisuals` delegates to `Animatable::updateVisualState` -- this advances the animation frame and syncs the sprite.
- `onRenderVisual` delegates to `Animatable::renderVisualState` -- this draws the sprite at the physics body position.

---

### Step 2 — Define the Animation Set

Add a new factory function in `Animation.h` / `Animation.cpp`.

**`include/Animation/Animation.h`** -- add declaration:

```cpp
namespace Animation {
    AnimationClip createLinearClip(...);
    AnimationSet makeDefaultPlayerAnimationSet();
    AnimationSet makeGoombaAnimationSet();
}
```

**`src/Animation/Animation.cpp`** -- add implementation:

```cpp
AnimationSet Animation::makeGoombaAnimationSet() {
    AnimationSet set;
    set.defaultClip = "walk";

    set.clips.emplace(
        "walk",
        Animation::createLinearClip(
            {0, 0},       // start position on spritesheet (pixels)
            {32, 32},     // frame size
            3,            // frame count
            {32, 0},      // stride between frames
            1.0f / 6.0f,  // frame duration (seconds)
            true          // looping
        )
    );

    set.clips.emplace(
        "dead",
        Animation::createLinearClip(
            {96, 0},
            {32, 32},
            1,
            {32, 0},
            0.5f,
            false
        )
    );

    return set;
}
```

**How `createLinearClip` works:**

| Parameter | Meaning |
|---|---|
| `startPosition` | Top-left corner of first frame on the spritesheet (pixels) |
| `frameSize` | Width / height of each frame |
| `frameCount` | Number of frames |
| `frameStride` | Pixel offset between consecutive frames |
| `frameDuration` | How long each frame is shown (seconds) |
| `looping` | Whether the clip restarts after the last frame |

---

### Step 3 — Register in AnimationLibrary

**`include/Animation/AnimationLibrary.h`** -- add declaration:

```cpp
private:
    void preloadPlayerAnimationSets();
    void preloadEnemyAnimationSets();
```

**`src/Animation/AnimationLibrary.cpp`** -- add implementation and call from constructor:

```cpp
#include "Animation/Animation.h"

AnimationLibrary::AnimationLibrary() {
    preloadPlayerAnimationSets();
    preloadEnemyAnimationSets();
}

void AnimationLibrary::preloadEnemyAnimationSets() {
    registerAnimationSet("goomba", Animation::makeGoombaAnimationSet());
}
```

---

### Step 4 — Register in GameObjectFactory

**`src/Game/Objects/GameObjectFactory.cpp`** -- add in constructor:

```cpp
#include "Game/Objects/Enemy/Goomba.h"

registerEnemy("Goomba", [](sf::Texture* texture) -> std::shared_ptr<GameObject> {
    if (texture) {
        return std::make_shared<Goomba>(*texture);
    }
    return std::make_shared<Goomba>();
});
```

---

### Step 5 — Load the Spritesheet in ResourceManager

**`src/ResourceManager.cpp`** -- add in constructor:

```cpp
_preLoadTexture("assets/spritesheets/goomba_spritesheet.png", "goomba_spritesheet");
```

Place your spritesheet file at `assets/spritesheets/goomba_spritesheet.png`.

---

### Step 6 — Spawn in GameWorld

**A) Spawn from map data** -- assign a new tile ID (e.g. `4 = goomba`) and add a branch in `loadMap`:

```cpp
else if (blockId == 4) {
    auto& goombaTexture = ResourceManager::getInstance().getTexture("goomba_spritesheet");
    auto goomba = _objectFactory.createEnemy("Goomba", &goombaTexture);
    goomba->spawn(_physicsWorld, spawnPos, {64, 64});
    _objects.push_back(goomba);
}
```

**B) Spawn programmatically** (e.g. in `GameWorld::test()`):

```cpp
auto& goombaTexture = ResourceManager::getInstance().getTexture("goomba_spritesheet");
auto goomba = _objectFactory.createEnemy("Goomba", &goombaTexture);
goomba->spawn(_physicsWorld, {500.f, 300.f}, {64, 64});
_objects.push_back(goomba);
```

---

### Step 7 — (Optional) Custom Behaviour

| Method | When to override |
|---|---|
| `updateSimulation` | Custom physics / AI movement per tick |
| `onUpdateVisuals` | Switch animation clip based on state |
| `onCreateBodyDef` | Change body type, gravity scale, fixed rotation, etc. |
| `onCreateShapeDef` | Change density, friction, restitution, sensor flag |

Example -- play death animation then self-destruct:

```cpp
void Goomba::updateSimulation(const float& fixedDt) {
    if (!isAlive() && !_pendingDestroy) {
        playAnimation("dead");
        // destroy after animation finishes
    }
}
```

---

### Checklist

- [ ] Header + source files created in `Enemy/` directory
- [ ] Constructor calls `configureVisuals(texture, "animation_set_id")`
- [ ] `onUpdateVisuals` calls `updateVisualState(deltaTime, _hitboxPixels, facingLeft)`
- [ ] `onRenderVisual` calls `renderVisualState(target, position, angleDegrees)`
- [ ] Animation set registered in `AnimationLibrary`
- [ ] Creator lambda registered in `GameObjectFactory`
- [ ] Spritesheet loaded in `ResourceManager` constructor
- [ ] Object spawned in `GameWorld` (map data or programmatic)
