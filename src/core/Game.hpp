#pragma once
#include <SFML/Graphics/RenderWindow.hpp>
#include "SceneManager.hpp"
#include "EventBus.hpp"
#include "GameState.hpp"

class Game {
public:
    Game();
    void run();

    sf::RenderWindow& window() { return _window; }
    SceneManager& sceneManager() { return _scenes; }
    EventBus& bus() { return _bus; }
    GameState& state() { return _state; }

private:
    sf::RenderWindow _window;
    SceneManager _scenes;
    EventBus _bus;
    GameState _state;

    void processEvents();
    void update(float dt);
    void render();
};
