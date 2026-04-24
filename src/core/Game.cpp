#include "Game.hpp"
#include "Constants.hpp"
#include <SFML/System/Clock.hpp>

Game::Game()
    : _window(sf::VideoMode({Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT}), Constants::WINDOW_TITLE)
{
    _window.setFramerateLimit(Constants::TARGET_FPS);
}

void Game::run() {
    sf::Clock clock;
    while (_window.isOpen()) {
        float dt = clock.restart().asSeconds();
        if (dt > 0.05f) dt = 0.05f;
        (void)dt;
        processEvents();
        render();
    }
}

void Game::processEvents() {
    while (const auto event = _window.pollEvent()) {
        if (event->is<sf::Event::Closed>())
            _window.close();
    }
}

void Game::render() {
    _window.clear(Constants::BACKGROUND_COLOR);
    _window.display();
}
