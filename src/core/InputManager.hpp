#pragma once
#include "config/GameplayConfig.hpp"
#include "config/InputConfig.hpp"
#include <SFML/System/Vector2.hpp>
#include <array>
#include <cmath>

struct PlayerInput {
    sf::Vector2f moveDir;
    bool shoot;
};

class InputManager {
public:
    void update() {
        for (int i = 0; i < GameplayConfig::PLAYER_COUNT; ++i)
            _inputs[i] = readPlayer(InputConfig::PLAYER_KEYS[i]);
    }

    const PlayerInput& get(int playerId) const { return _inputs[playerId]; }

private:
    std::array<PlayerInput, GameplayConfig::PLAYER_COUNT> _inputs{};

    static PlayerInput readPlayer(const InputConfig::KeyBindings& keys) {
        PlayerInput input{};
        sf::Vector2f direction{};
        if (sf::Keyboard::isKeyPressed(keys.up)) direction.y -= 1.f;
        if (sf::Keyboard::isKeyPressed(keys.down)) direction.y += 1.f;
        if (sf::Keyboard::isKeyPressed(keys.left)) direction.x -= 1.f;
        if (sf::Keyboard::isKeyPressed(keys.right)) direction.x += 1.f;
        float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        if (length > 0.f)
            direction /= length;
        input.moveDir = direction;
        input.shoot = sf::Keyboard::isKeyPressed(keys.shoot);
        return input;
    }
};
