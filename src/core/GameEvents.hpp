#pragma once
#include <SFML/System/Vector2.hpp>

// whoever is gonna take care of powers can change this freely, these are just exemples I thought of
enum class PowerUpType {
    SpeedBoost,
    RapidFire,
    SpreadShot,
    Shield,
    HomingBullets,
    Invincibility,
    TripleDamage,
};

// Events are plain data: modules emit and listen without direct interactions (see EventBus.hpp).
struct PlayerDiedEvent { int victimId; int killerId; sf::Vector2f position; };
struct PlayerHitEvent { int victimId; int killerId; };
struct DiceRolledEvent { int playerId; int value; };
struct PowerUpSpawnedEvent { sf::Vector2f position; PowerUpType type; int tier; };
struct PowerUpPickedEvent { int playerId; PowerUpType type; float duration; };
struct PowerUpExpiredEvent { int playerId; PowerUpType type; };
struct PlayerRespawnEvent { int playerId; sf::Vector2f position; };
struct GameWonEvent { int winnerId; int kills; };
struct VFXRequestEvent {
    enum class Type {
        Explosion,
        DiceRoll,
        Pickup
    } type;
    sf::Vector2f position;
};
