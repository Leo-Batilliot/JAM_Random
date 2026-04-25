#pragma once

namespace DecorConfig {
    // Floor texture tiling
    inline constexpr float FLOOR_TILE_SIZE = 32.f;

    // Wall face strips
    inline constexpr float WALL_TILE_SIZE  = 48.f;

    // Large rocks placed first to claim the best spots
    inline constexpr float ROCK_LARGE_SCALE          = 0.065f;
    inline constexpr float ROCK_LARGE_COLLISION_RADIUS = 52.f;
    inline constexpr int   ROCK_LARGE_COUNT_MIN      = 1;
    inline constexpr int   ROCK_LARGE_COUNT_MAX      = 3;

    // Small rocks
    inline constexpr float ROCK_SMALL_SCALE            = 0.022f;
    inline constexpr float ROCK_SMALL_COLLISION_RADIUS = 18.f;
    inline constexpr int   ROCK_SMALL_COUNT_MIN        = 10;
    inline constexpr int   ROCK_SMALL_COUNT_MAX        = 20;

    // Placement constraints
    inline constexpr float ROCK_SPAWN_MARGIN = 30.f;
    inline constexpr float ROCK_MIN_GAP      = 12.f;
    inline constexpr int   ROCK_PLACE_ATTEMPTS = 60;
}
