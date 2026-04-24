#include "Game.hpp"
#include "Constants.hpp"
#include <SFML/System/Clock.hpp>
#include "../world/MenuScene.hpp"

Game::Game()
    : _window(sf::VideoMode({Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT}), Constants::WINDOW_TITLE)
{
    _window.setFramerateLimit(Constants::TARGET_FPS);
    _scenes.push(std::make_unique<MenuScene>(*this));
    _scenes.applyPending();
}

void Game::run() {
    sf::Clock clock;
    while (_window.isOpen() && !_scenes.empty()) {
        float dt = clock.restart().asSeconds();
        if (dt > Constants::MAX_FRAME_DELTA)
            dt = Constants::MAX_FRAME_DELTA;
        processEvents();
        update(dt);
        render();
        _scenes.applyPending();
    }
}

void Game::processEvents() {
    while (const auto event = _window.pollEvent()) {
        if (event->is<sf::Event::Closed>())
            _window.close();
        if (auto* scene = _scenes.current())
            scene->handleEvent(*event, _scenes);
    }
}

void Game::update(float dt) {
    if (auto* s = _scenes.current())
        s->update(dt, _scenes);
}

void Game::render() {
    _window.clear(Constants::BACKGROUND_COLOR);
    if (auto* s = _scenes.current())
        s->render(_window);
    _window.display();
}
