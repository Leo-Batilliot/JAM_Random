#pragma once
#include <SFML/Graphics/RenderWindow.hpp>

class Game {
public:
    Game();
    void run();

    sf::RenderWindow& window() { return _window; }

private:
    sf::RenderWindow _window;

    void processEvents();
    void render();
};
