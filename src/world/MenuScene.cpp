#include "MenuScene.hpp"
#include "../core/Game.hpp"
#include "../core/Constants.hpp"
#include <SFML/Graphics.hpp>

MenuScene::MenuScene(Game& game) : _game(game) {}

void MenuScene::handleEvent(const sf::Event& event, SceneManager& sm) {
    (void)event;
    (void)sm;
}

void MenuScene::update(float, SceneManager&) {}

void MenuScene::render(sf::RenderWindow& window) {
    sf::RectangleShape bg(sf::Vector2f(window.getSize()));
    bg.setFillColor(Constants::MENU_BACKGROUND_COLOR);
    window.draw(bg);
}
