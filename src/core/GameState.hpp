#pragma once
#include <array>
#include <vector>
#include <SFML/System/Vector2.hpp>
#include "config/GameplayConfig.hpp"
#include "EventBus.hpp"
#include "GameEvents.hpp"

// Active effect
struct ActiveEffect {
    PowerUpType type;
    float timeRemaining;
};

// Player data
struct PlayerState {
    int kills = 0;
    int hp = GameplayConfig::PLAYER_MAX_HP;
    bool alive = true;

    float respawnTimer = 0.f;
    bool invincible = false;
    float invincibleTimer = 0.f;

    sf::Vector2f spawnPosition;
    sf::Vector2f position;
    float radius = GameplayConfig::PLAYER_RADIUS;
    std::vector<ActiveEffect> effects;

    bool hasEffect(PowerUpType type) const {
        for (auto& effect : effects) {
            if (effect.type == type)
                return true;
        }
        return false;
    }

    void addEffect(PowerUpType type, float duration) {
        for (auto& effect : effects) {
            if (effect.type == type) {
                effect.timeRemaining = duration;
                return;
            }
        }
        effects.push_back({type, duration});
    }

    // Counts down effects and emits PowerUpExpiredEvent when a timer ends.
    void tickEffects(float dt, EventBus& bus, int playerId) {
        for (int i = static_cast<int>(effects.size()) - 1; i >= 0; --i) {
            effects[i].timeRemaining -= dt;
            if (effects[i].timeRemaining <= 0.f) {
                bus.emit(PowerUpExpiredEvent{playerId, effects[i].type});
                effects.erase(effects.begin() + i);
            }
        }
    }
};

// Global game state
struct GameState {
    std::array<PlayerState, GameplayConfig::PLAYER_COUNT> players;
    bool gameOver = false;
    int winnerId = -1;

    void reset(sf::Vector2f spawn0, sf::Vector2f spawn1) {
        players[0] = PlayerState{};
        players[1] = PlayerState{};
        players[0].spawnPosition = spawn0;
        players[1].spawnPosition = spawn1;
        players[0].position = spawn0;
        players[1].position = spawn1;
        players[0].radius = GameplayConfig::PLAYER_RADIUS;
        players[1].radius = GameplayConfig::PLAYER_RADIUS;
        gameOver = false;
        winnerId = -1;
    }
};
