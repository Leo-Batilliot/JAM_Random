#include "Game.hpp"
#include "config/ColorConfig.hpp"
#include "config/WindowConfig.hpp"
#include <SFML/System/Clock.hpp>
#include "../world/MenuScene.hpp"

Game::Game()
    : _window(sf::VideoMode({WindowConfig::WINDOW_WIDTH, WindowConfig::WINDOW_HEIGHT}), WindowConfig::WINDOW_TITLE, sf::Style::None)
{
    _window.setFramerateLimit(WindowConfig::TARGET_FPS);
    
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    _window.setPosition({
        static_cast<int>((desktop.size.x - _window.getSize().x) / 2),
        static_cast<int>((desktop.size.y - _window.getSize().y) / 2)
    });

    _scenes.push(std::make_unique<MenuScene>(*this));
    _scenes.applyPending();
}

void Game::run() {
    sf::Clock clock;
    while (_window.isOpen() && !_scenes.empty()) {
        float dt = clock.restart().asSeconds();
        if (dt > WindowConfig::MAX_FRAME_DELTA)
            dt = WindowConfig::MAX_FRAME_DELTA;
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
    _input.update();
    if (auto* scene = _scenes.current())
        scene->update(dt, _scenes);
}

void Game::render() {
    _window.clear(ColorConfig::BACKGROUND_COLOR);
    if (auto* scene = _scenes.current())
        scene->render(_window);
    _window.display();
}
