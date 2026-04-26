#pragma once

#include "GameplayConfig.hpp"
#include <SFML/Window/Keyboard.hpp>
#include <array>

namespace InputConfig {
struct KeyBindings {
    sf::Keyboard::Key up;
    sf::Keyboard::Key down;
    sf::Keyboard::Key left;
    sf::Keyboard::Key right;
    sf::Keyboard::Key shoot;
    sf::Keyboard::Key shootAlt = sf::Keyboard::Key::Unknown;
};

// Change bindings here if you want different keys.
inline constexpr std::array<KeyBindings, GameplayConfig::PLAYER_COUNT> PLAYER_KEYS{{
    {sf::Keyboard::Key::Z, sf::Keyboard::Key::S, sf::Keyboard::Key::Q, sf::Keyboard::Key::D, sf::Keyboard::Key::Space, sf::Keyboard::Key::Unknown},
    {sf::Keyboard::Key::Up, sf::Keyboard::Key::Down, sf::Keyboard::Key::Left, sf::Keyboard::Key::Right, sf::Keyboard::Key::Enter, sf::Keyboard::Key::RShift}
}};
}
