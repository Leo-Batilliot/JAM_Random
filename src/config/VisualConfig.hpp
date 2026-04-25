#pragma once
#include <cstdint>

namespace VisualConfig {
    // Character sprite 
    inline constexpr float CHARACTER_SCALE           = 0.22f;
    inline constexpr float CHARACTER_Y_ORIGIN_FACTOR = 0.74f;
    // invincibility blink
    inline constexpr uint8_t INVINCIBLE_BLINK_LOW_ALPHA = 55;

    // Weapon sprite
    inline constexpr float WEAPON_SCALE    = 0.16f;
    inline constexpr float WEAPON_OFFSET_X = 12.f;
    inline constexpr float WEAPON_OFFSET_Y = 8.f;

    // Muzzle flash
    inline constexpr float MUZZLE_SCALE          = 0.14f;
    inline constexpr float MUZZLE_OFFSET_DISTANCE = 28.f;
    inline constexpr float MUZZLE_FLASH_DURATION  = 0.08f;
    inline constexpr uint8_t MUZZLE_FLASH_MAX_ALPHA = 220;

    // Hit spark
    inline constexpr float HIT_EFFECT_DURATION  = 0.35f;
    inline constexpr float HIT_EFFECT_MAX_RADIUS = 20.f;
    inline constexpr float HIT_FLASH_DURATION    = 0.3f;
    inline constexpr uint8_t HIT_FLASH_MAX_ALPHA = 110;
    inline constexpr uint8_t HIT_SPARK_MAX_ALPHA = 160;

    // Bullet
    inline constexpr float BULLET_OUTLINE_THICKNESS = 1.5f;
}
