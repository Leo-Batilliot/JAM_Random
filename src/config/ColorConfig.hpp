#pragma once
#include <SFML/Graphics/Color.hpp>

namespace ColorConfig {
    // Core
    inline constexpr sf::Color BACKGROUND_COLOR = sf::Color(30, 30, 40);
    inline constexpr sf::Color MENU_BACKGROUND_COLOR = sf::Color(20, 20, 30);
    inline constexpr sf::Color ARENA_OUTLINE_COLOR = sf::Color(255, 255, 255);

    // Players
    inline constexpr sf::Color PLAYER0_COLOR = sf::Color(90, 170, 255);
    inline constexpr sf::Color PLAYER1_COLOR = sf::Color(255, 120, 120);

    // Bullets
    inline constexpr sf::Color BULLET_P0_FILL = sf::Color(140, 200, 255);
    inline constexpr sf::Color BULLET_P0_OUTLINE = sf::Color(70, 140, 215);
    inline constexpr sf::Color BULLET_P1_FILL = sf::Color(255, 160, 160);
    inline constexpr sf::Color BULLET_P1_OUTLINE = sf::Color(215, 85, 85);

    // Arena decor
    inline constexpr sf::Color FLOOR_TINT = sf::Color(170, 175, 185);
    inline constexpr sf::Color WALL_FACE_COLOR = sf::Color(140, 100, 60);

    // VFX
    inline constexpr sf::Color HIT_SPARK_COLOR = sf::Color(255, 200, 50);
    inline constexpr sf::Color MUZZLE_FLASH_COLOR = sf::Color(255, 220, 100);

    // HUD
    inline constexpr sf::Color HUD_PANEL_BG = sf::Color(0, 0, 0, 150);
    inline constexpr sf::Color HUD_HP_BAR_BG = sf::Color(40, 40, 40);
    inline constexpr sf::Color HUD_HP_BAR_BORDER = sf::Color(85, 85, 85);
    inline constexpr sf::Color HUD_LABEL_GRAY = sf::Color(160, 160, 160);
    inline constexpr sf::Color HUD_SCORE_LABEL = sf::Color(150, 150, 150);
    inline constexpr sf::Color HUD_SCORE_VALUE = sf::Color(215, 215, 215);
    inline constexpr sf::Color HUD_EFFECT_LABEL = sf::Color(175, 175, 175);
    inline constexpr sf::Color HUD_HP_EMPTY = sf::Color(45, 45, 45);
    inline constexpr sf::Color HUD_HP_EMPTY_BORDER = sf::Color(85, 85, 85);
}
