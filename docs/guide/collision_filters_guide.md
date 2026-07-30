# Collision Filter (Category / Mask) Bit Rules

## Bit Allocations

| Bit | Category | Tag | Assigned To |
| --- | -------- | --- | ----------- |
| `0x0001` | Environment | `ENV` | Terrain tiles, blocks, walls, ground, player feet sensor |
| `0x0002` | Player | `PLAYER` | Player hitbox |
| `0x0004` | Fireball | `FIREBALL` | Player fireballs |
| `0x0008` | Enemy | `ENEMY` | Goomba, Koopa, and other enemies |
| `0x0010` | Pickup | `PICKUP` | FireFlower, KoopaShell, and future collectible items |

Bits `0x0020` through `0x8000` are available for future categories.

---

## Filter Table

| ↓ Object (category) / → Interacts with | `0x0001` ENV | `0x0002` PLAYER | `0x0004` FIREBALL | `0x0008` ENEMY | `0x0010` PICKUP |
|---|---|---|---|---|---|
| **Environment** — default mask | ✓ | ✓ | ✓ | ✓ | ✓ |
| **Player** — mask `0x0001\|0x0008\|0x0010` | ✓ | ❌ | ❌ | ✓ | ✓ |
| **Fireball**— mask `0x0001\|0x0008` | ✓ | ❌ | ❌ | ✓ | ❌ |
| **Enemy**— default mask | ✓ | ✓ | ✓ | ✓ | ✓ |
| **Pickup** — mask `0x0002` | ❌ | ✓ | ❌ | ❌ | ❌ |