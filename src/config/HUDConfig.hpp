#pragma once
#include <cstdint>

namespace HUDConfig {
    // Panel geometry
    inline constexpr float PANEL_WIDTH       = 260.f;
    inline constexpr float PANEL_HEIGHT      = 128.f;
    inline constexpr float PANEL_Y           = 20.f;
    inline constexpr float PANEL_SIDE_MARGIN = 20.f;
    inline constexpr float PANEL_OUTLINE_ALPHA = 180.f;
    inline constexpr float PANEL_BG_ALPHA    = 150.f;

    // HP bar
    inline constexpr float HP_BAR_OFFSET_X = 42.f;
    inline constexpr float HP_BAR_WIDTH     = 198.f;
    inline constexpr float HP_BAR_HEIGHT    = 12.f;

    // Layout — Y offsets from panel top
    inline constexpr float PLAYER_LABEL_Y = 8.f;
    inline constexpr float HP_ROW_Y       = 34.f;
    inline constexpr float SCORE_LABEL_Y  = 58.f;
    inline constexpr float SCORE_VALUE_Y  = 74.f;
    inline constexpr float EFFECT_ROW_Y   = 102.f;

    // Effect icons
    inline constexpr float EFFECT_DOT_RADIUS = 5.f;
    inline constexpr float EFFECT_SPACING    = 52.f;

    // Font sizes
    inline constexpr unsigned int FONT_PLAYER_LABEL = 12;
    inline constexpr unsigned int FONT_HP_LABEL     = 9;
    inline constexpr unsigned int FONT_SCORE_LABEL  = 8;
    inline constexpr unsigned int FONT_SCORE_VALUE  = 14;
    inline constexpr unsigned int FONT_EFFECT_LABEL = 7;
}
