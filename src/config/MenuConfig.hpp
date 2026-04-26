#pragma once
#include <cstdint>

namespace MenuConfig {
    inline constexpr float BORDER_INSET   = 20.f;
    inline constexpr float BORDER_ALPHA   = 55.f;
    inline constexpr float DIVIDER_WIDTH  = 420.f;
    inline constexpr float DIVIDER_ALPHA  = 55.f;

    // Vertical layout — as a fraction of window height
    inline constexpr float TITLE_LINE1_Y  = 0.28f;
    inline constexpr float TITLE_LINE2_Y  = 0.39f;
    inline constexpr float DIVIDER_Y      = 0.51f;
    inline constexpr float CONTROLS_P1_Y  = 0.59f;
    inline constexpr float CONTROLS_P2_Y  = 0.68f;
    inline constexpr float PROMPT_Y       = 0.83f;
    inline constexpr float BLINK_PERIOD   = 0.48f;

    inline constexpr unsigned int FONT_TITLE    = 64;
    inline constexpr unsigned int FONT_CONTROLS = 13;
    inline constexpr unsigned int FONT_PROMPT   = 18;

    // Win screen
    inline constexpr float WIN_PLAYER_LABEL_Y = 0.32f;
    inline constexpr float WIN_WINS_Y         = 0.45f;
    inline constexpr float WIN_SCORE_Y        = 0.60f;
    inline constexpr float WIN_TAUNT_Y        = 0.72f;
    inline constexpr float WIN_PROMPT_Y       = 0.86f;
    inline constexpr float WIN_TINT_ALPHA     = 16.f;
    inline constexpr float WIN_BORDER_ALPHA   = 210.f;
    inline constexpr float WIN_BORDER_THICKNESS = 4.f;

    inline constexpr unsigned int FONT_WIN_PLAYER = 36;
    inline constexpr unsigned int FONT_WIN_WINS   = 58;
    inline constexpr unsigned int FONT_WIN_SCORE  = 28;
    inline constexpr unsigned int FONT_WIN_TAUNT  = 16;
    inline constexpr unsigned int FONT_WIN_PROMPT = 14;
}
