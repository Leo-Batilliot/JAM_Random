#pragma once

#include <SFML/Graphics/Color.hpp>
#include <array>

namespace PowerUpConfig {
    // Spawn
    inline constexpr float MIN_SPAWN_INTERVAL = 5.f;
    inline constexpr float MAX_SPAWN_INTERVAL = 15.f;

    // Behavior
    inline constexpr float POWERUP_LIFETIME = 10.f;
    inline constexpr float POWERUP_RADIUS = 16.f;
    inline constexpr int POWERUP_MAX_ON_MAP = 3;
    inline constexpr float POWERUP_EFFECT_DURATION = 8.f;

    inline constexpr std::array<float, 3> TIER_DURATION_MULTIPLIERS = {
        1.0f,
        1.5f,
        2.0f
    };

    inline constexpr std::array<sf::Color, 3> TIER_COLORS = {
        sf::Color(140, 230, 160),
        sf::Color(255, 210, 120),
        sf::Color(255, 140, 140)
    };
}
