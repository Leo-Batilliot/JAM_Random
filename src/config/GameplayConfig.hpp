#pragma once

namespace GameplayConfig {
    inline constexpr int KILLS_TO_WIN = 5;
    inline constexpr int PLAYER_COUNT = 2;

    inline constexpr float PLAYER_SPEED = 200.f;
    inline constexpr float PLAYER_RADIUS = 18.f;
    inline constexpr int PLAYER_MAX_HP = 3;
    inline constexpr float PLAYER_INVINCIBILITY_DURATION = 1.5f;
    inline constexpr float PLAYER_RESPAWN_DELAY = 2.f;

    inline constexpr float BULLET_SPEED = 500.f;
    inline constexpr float BULLET_RADIUS = 5.f;
    inline constexpr float BULLET_LIFETIME = 2.f;
    inline constexpr float FIRE_COOLDOWN = 0.4f;
}
