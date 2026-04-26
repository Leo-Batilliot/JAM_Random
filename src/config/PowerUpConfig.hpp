#pragma once

#include <SFML/Graphics/Color.hpp>
#include <array>
#include <string_view>

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

    // Gameplay tuning for effects used by CombatModule/PowerUpModule.
    inline constexpr float SPEED_BOOST_MULTIPLIER = 1.35f;
    inline constexpr float RAPID_FIRE_COOLDOWN_MULTIPLIER = 0.55f;
    inline constexpr float MIN_FIRE_COOLDOWN = 0.08f;
    inline constexpr int TRIPLE_DAMAGE_BULLET_DAMAGE = 3;
    inline constexpr int NORMAL_BULLET_DAMAGE = 1;
    inline constexpr int SPREAD_PROJECTILE_COUNT = 3;
    inline constexpr float SPREAD_ANGLE_DEGREES = 14.f;
    inline constexpr float HOMING_TURN_RATE = 7.f;
    inline constexpr float INVINCIBILITY_REFRESH_TIME = 0.1f;

    // One icon per power-up type for instant recognition in-game.
    inline constexpr std::array<std::string_view, 7> TYPE_ICON_TEXTURE_PATHS = {
        "assets/textures/powerup_speedboost.png",
        "assets/textures/powerup_rapidfire.png",
        "assets/textures/powerup_spreadshot.png",
        "assets/textures/powerup_shield.png",
        "assets/textures/powerup_homing.png",
        "assets/textures/powerup_invincibility.png",
        "assets/textures/powerup_tripledamage.png"
    };

    // Type tint overlay to distinguish power-up families quickly.
    inline constexpr std::array<sf::Color, 7> TYPE_TINT_COLORS = {
        sf::Color(110, 220, 255),
        sf::Color(255, 210, 110),
        sf::Color(220, 170, 255),
        sf::Color(120, 255, 170),
        sf::Color(255, 140, 210),
        sf::Color(255, 255, 255),
        sf::Color(255, 120, 120)
    };
}
