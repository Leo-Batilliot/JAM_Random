#include "MenuScene.hpp"
#include "GameScene.hpp"
#include "../core/Game.hpp"
#include "../core/AssetManager.hpp"
#include "config/AssetConfig.hpp"
#include "config/ColorConfig.hpp"
#include "config/MenuConfig.hpp"
#include "config/WindowConfig.hpp"
#include <SFML/Graphics.hpp>

MenuScene::MenuScene(Game& game) : _game(game) {}

void MenuScene::handleEvent(const sf::Event& event, SceneManager& sceneManager) {
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (key->code == sf::Keyboard::Key::Escape) {
            sceneManager.pop();
            return;
        }
        if (key->code == sf::Keyboard::Key::Enter) {
            sceneManager.replace(std::make_unique<GameScene>(_game));
            return;
        }
    }
}

void MenuScene::update(float dt, SceneManager&) {
    _blinkTimer += dt;
    if (_blinkTimer >= MenuConfig::BLINK_PERIOD) {
        _blinkTimer -= MenuConfig::BLINK_PERIOD;
        _promptVisible = !_promptVisible;
    }
}

void MenuScene::render(sf::RenderWindow& window) {
    const auto& font = AssetManager::instance().font(AssetConfig::FONT);
    const float width  = static_cast<float>(WindowConfig::WINDOW_WIDTH);
    const float height = static_cast<float>(WindowConfig::WINDOW_HEIGHT);
    const float centerX = width / 2.f;

    // Background
    sf::RectangleShape background({width, height});
    background.setFillColor(ColorConfig::MENU_BACKGROUND_COLOR);
    window.draw(background);

    // Border
    sf::RectangleShape border({width - 2.f * MenuConfig::BORDER_INSET, height - 2.f * MenuConfig::BORDER_INSET});
    border.setPosition({MenuConfig::BORDER_INSET, MenuConfig::BORDER_INSET});
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineColor(sf::Color(255, 215, 0, static_cast<uint8_t>(MenuConfig::BORDER_ALPHA)));
    border.setOutlineThickness(2.f);
    window.draw(border);

    // Centers text horizontally at the given Y position
    auto centerAt = [&](sf::Text& text, float y) {
        const auto bounds = text.getLocalBounds();
        text.setOrigin({bounds.position.x + bounds.size.x * 0.5f, bounds.position.y + bounds.size.y * 0.5f});
        text.setPosition({centerX, y});
    };

    // Title
    sf::Text titleLine1(font, "DICE", MenuConfig::FONT_TITLE);
    titleLine1.setFillColor(ColorConfig::PLAYER0_COLOR);
    centerAt(titleLine1, height * MenuConfig::TITLE_LINE1_Y);
    window.draw(titleLine1);

    sf::Text titleLine2(font, "DUEL", MenuConfig::FONT_TITLE);
    titleLine2.setFillColor(ColorConfig::PLAYER1_COLOR);
    centerAt(titleLine2, height * MenuConfig::TITLE_LINE2_Y);
    window.draw(titleLine2);

    sf::RectangleShape divider({MenuConfig::DIVIDER_WIDTH, 1.f});
    divider.setOrigin({MenuConfig::DIVIDER_WIDTH / 2.f, 0.5f});
    divider.setPosition({centerX, height * MenuConfig::DIVIDER_Y});
    divider.setFillColor(sf::Color(255, 255, 255, static_cast<uint8_t>(MenuConfig::DIVIDER_ALPHA)));
    window.draw(divider);

    // Controls
    sf::Text controlsP1(font, "P1 : Z-Q-S-D  +  SPACE", MenuConfig::FONT_CONTROLS);
    controlsP1.setFillColor(ColorConfig::PLAYER0_COLOR);
    centerAt(controlsP1, height * MenuConfig::CONTROLS_P1_Y);
    window.draw(controlsP1);

    sf::Text controlsP2(font, "P2 : ARROWS  +  ENTER", MenuConfig::FONT_CONTROLS);
    controlsP2.setFillColor(ColorConfig::PLAYER1_COLOR);
    centerAt(controlsP2, height * MenuConfig::CONTROLS_P2_Y);
    window.draw(controlsP2);

    // Prompt
    if (_promptVisible) {
        sf::Text prompt(font, "PRESS ENTER TO PLAY", MenuConfig::FONT_PROMPT);
        prompt.setFillColor(sf::Color(240, 240, 240));
        centerAt(prompt, height * MenuConfig::PROMPT_Y);
        window.draw(prompt);
    }
}
