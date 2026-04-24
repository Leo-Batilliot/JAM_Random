#include "WinScene.hpp"
#include "MenuScene.hpp"
#include "../core/Game.hpp"

WinScene::WinScene(Game& game, int winnerId)
    : _game(game), _winnerId(winnerId) {}

void WinScene::handleEvent(const sf::Event& event, SceneManager& sm) {
    if (event.is<sf::Event::KeyPressed>()) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
            sm.pop();
        sm.replace(std::make_unique<MenuScene>(_game));
    }
}

void WinScene::update(float, SceneManager&) {}

void WinScene::render(sf::RenderWindow& window) {
    (void)window;
}
