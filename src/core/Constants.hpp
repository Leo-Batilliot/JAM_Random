#pragma once

#include <SFML/Graphics/Color.hpp>

// Everything is up to change depending on how we want to make things work in the end
namespace Constants {
    inline constexpr unsigned int WINDOW_WIDTH = 1920;
    inline constexpr unsigned int WINDOW_HEIGHT = 1080;
    inline constexpr unsigned int TARGET_FPS = 60;
    inline constexpr const char* WINDOW_TITLE = "Jam Random";

    inline constexpr int KILLS_TO_WIN = 5;
    inline constexpr int PLAYER_COUNT = 2;

    inline constexpr float PLAYER_SPEED = 200.f;
    inline constexpr float PLAYER_RADIUS = 18.f;
    inline constexpr int PLAYER_MAX_HP = 3;
    inline constexpr float PLAYER_INVINCIBILITY_DURATION = 1.5f;
    inline constexpr float PLAYER_RESPAWN_DELAY = 2.f;

    inline constexpr float BULLET_SPEED = 500.f;
    inline constexpr float BULLET_RADIUS = 5.0f;
    inline constexpr float BULLET_LIFETIME = 2.0f;
    inline constexpr float FIRE_COOLDOWN = 0.4f;

    inline constexpr sf::Color BACKGROUND_COLOR = sf::Color(30, 30, 40);

    inline constexpr float MIN_POWERUP_SPAWN_INTERVAL = 5.0f;
    inline constexpr float MAX_POWERUP_SPAWN_INTERVAL = 15.f;
    inline constexpr float POWERUP_LIFETIME           = 10.f;
    inline constexpr float POWERUP_RADIUS             = 16.f;
    inline constexpr int   POWERUP_MAX_ON_MAP         = 3;
    inline constexpr float POWERUP_EFFECT_DURATION    = 8.f;

    inline constexpr float WALL_THICKNESS = 32.f;
}
