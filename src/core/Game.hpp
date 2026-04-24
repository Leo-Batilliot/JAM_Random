#pragma once
#include <SFML/Graphics/RenderWindow.hpp>
#include "SceneManager.hpp"

class Game {
public:
    Game();
    void run();

    sf::RenderWindow& window() { return _window; }
    SceneManager& sceneManager() { return _scenes; }

private:
    sf::RenderWindow _window;
    SceneManager _scenes;

    void processEvents();
    void update(float dt);
    void render();
};
