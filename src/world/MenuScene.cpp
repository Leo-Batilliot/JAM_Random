#include "MenuScene.hpp"
#include "GameScene.hpp"
#include "../core/Game.hpp"
#include "config/ColorConfig.hpp"
#include <SFML/Graphics.hpp>

MenuScene::MenuScene(Game& game) : _game(game) {}

void MenuScene::handleEvent(const sf::Event& event, SceneManager& sm) {
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (key->code == sf::Keyboard::Key::Escape)
            sm.pop();
        if (key->code == sf::Keyboard::Key::Enter)
            sm.replace(std::make_unique<GameScene>(_game));
    }
}

void MenuScene::update(float, SceneManager&) {}

void MenuScene::render(sf::RenderWindow& window) {
    sf::RectangleShape bg(sf::Vector2f(window.getSize()));
    bg.setFillColor(ColorConfig::MENU_BACKGROUND_COLOR);
    window.draw(bg);
}
