#pragma once

class Damageable {
public:
    Damageable(int initHealth = 100);
    virtual ~Damageable() = default;

    void takeDamage(int amount);
    void heal(int amount);

    bool isAlive() const { return _health > 0; }
    int getCurrentHealth() const { return _health; }
    int getMaxHealth() const { return _maxHealth; }

protected:
    int _maxHealth = 100;
    int _health = 100;
};