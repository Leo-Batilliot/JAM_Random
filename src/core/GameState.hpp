#pragma once
#include <array>
#include <vector>
#include <SFML/System/Vector2.hpp>
#include "Constants.hpp"
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
    int hp = Constants::PLAYER_MAX_HP;
    bool alive = true;

    float respawnTimer = 0.f;
    bool invincible = false;
    float invincibleTimer = 0.f;

    sf::Vector2f spawnPosition;
    std::vector<ActiveEffect> effects;

    bool hasEffect(PowerUpType t) const {
        for (auto& effect : effects) {
            if (effect.type == t)
                return true;
        }
        return false;
    }

    void addEffect(PowerUpType t, float duration) {
        for (auto& effect : effects) {
            if (effect.type == t) {
                effect.timeRemaining = duration;
                return;
            }
        }
        effects.push_back({t, duration});
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
    std::array<PlayerState, Constants::PLAYER_COUNT> players;
    bool gameOver = false;
    int winnerId = -1;

    void reset(sf::Vector2f spawn0, sf::Vector2f spawn1) {
        players[0] = PlayerState{};
        players[1] = PlayerState{};
        players[0].spawnPosition = spawn0;
        players[1].spawnPosition = spawn1;
        gameOver = false;
        winnerId = -1;
    }
};
