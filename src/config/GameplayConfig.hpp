#pragma once

namespace GameplayConfig {
    // Match rules
    inline constexpr int KILLS_TO_WIN = 3;
    inline constexpr int PLAYER_COUNT = 2;

    // Player
    inline constexpr float PLAYER_SPEED = 300.f;
    inline constexpr float PLAYER_RADIUS = 18.f;
    inline constexpr int PLAYER_MAX_HP = 3;
    inline constexpr float PLAYER_INVINCIBILITY_DURATION = 0.35f;
    inline constexpr float PLAYER_RESPAWN_DELAY = 2.f;

    // Weapons
    inline constexpr float BULLET_SPEED = 650.f;
    inline constexpr float BULLET_RADIUS = 5.f;
    inline constexpr float BULLET_LIFETIME = 2.f;
    inline constexpr float FIRE_COOLDOWN = 0.35f;

    // Spawn offset for gun position
    inline constexpr float BULLET_SPAWN_OFFSET_X = 12.f;
    inline constexpr float BULLET_SPAWN_OFFSET_Y = 8.f;
}
