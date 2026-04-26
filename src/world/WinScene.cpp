#include "WinScene.hpp"
#include "MenuScene.hpp"
#include "../core/Game.hpp"
#include "../core/AssetManager.hpp"
#include "config/AssetConfig.hpp"
#include "config/ColorConfig.hpp"
#include "config/GameplayConfig.hpp"
#include "config/MenuConfig.hpp"
#include "config/WindowConfig.hpp"
#include <SFML/Graphics.hpp>
#include <string>

WinScene::WinScene(Game& game, int winnerId)
    : _game(game), _winnerId(winnerId),
      _taunt("MAYBE PLAYER " + std::to_string(2 - winnerId) + " SHOULD BE PLAYING TIBOUDADA INSTEAD") {}

void WinScene::handleEvent(const sf::Event& event, SceneManager& sceneManager) {
    if (event.is<sf::Event::KeyPressed>()) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
            sceneManager.pop();
            return;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter)) {
            sceneManager.replace(std::make_unique<MenuScene>(_game));
            return;
        }
    }
}

void WinScene::update(float, SceneManager&) {}

void WinScene::render(sf::RenderWindow& window) {
    const auto& font = AssetManager::instance().font(AssetConfig::FONT);
    const float width = static_cast<float>(WindowConfig::WINDOW_WIDTH);
    const float height = static_cast<float>(WindowConfig::WINDOW_HEIGHT);
    const float centerX = width / 2.f;

    const sf::Color winnerColor = (_winnerId == 0) ? ColorConfig::PLAYER0_COLOR : ColorConfig::PLAYER1_COLOR;

    sf::RectangleShape background({width, height});
    background.setFillColor(sf::Color(14, 14, 20));
    window.draw(background);

    sf::RectangleShape tint({width, height});
    tint.setFillColor(sf::Color(winnerColor.r, winnerColor.g, winnerColor.b, static_cast<uint8_t>(MenuConfig::WIN_TINT_ALPHA)));
    window.draw(tint);

    sf::RectangleShape border({width - 2.f * MenuConfig::BORDER_INSET, height - 2.f * MenuConfig::BORDER_INSET});
    border.setPosition({MenuConfig::BORDER_INSET, MenuConfig::BORDER_INSET});
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineColor(sf::Color(winnerColor.r, winnerColor.g, winnerColor.b, static_cast<uint8_t>(MenuConfig::WIN_BORDER_ALPHA)));
    border.setOutlineThickness(MenuConfig::WIN_BORDER_THICKNESS);
    window.draw(border);

    auto centerAt = [&](sf::Text& text, float y) {
        const auto bounds = text.getLocalBounds();
        text.setOrigin({bounds.position.x + bounds.size.x * 0.5f, bounds.position.y + bounds.size.y * 0.5f});
        text.setPosition({centerX, y});
    };

    sf::Text playerLabel(font, "PLAYER " + std::to_string(_winnerId + 1), MenuConfig::FONT_WIN_PLAYER);
    playerLabel.setFillColor(winnerColor);
    centerAt(playerLabel, height * MenuConfig::WIN_PLAYER_LABEL_Y);
    window.draw(playerLabel);

    sf::Text winsText(font, "WINS!", MenuConfig::FONT_WIN_WINS);
    winsText.setFillColor(sf::Color(245, 245, 245));
    centerAt(winsText, height * MenuConfig::WIN_WINS_Y);
    window.draw(winsText);

    const int winnerKills = _game.state().players[_winnerId].kills;
    const int loserKills = _game.state().players[1 - _winnerId].kills;
    sf::Text scoreText(font, std::to_string(winnerKills) + "  -  " + std::to_string(loserKills), MenuConfig::FONT_WIN_SCORE);
    scoreText.setFillColor(sf::Color(195, 195, 195));
    centerAt(scoreText, height * MenuConfig::WIN_SCORE_Y);
    window.draw(scoreText);

    sf::Text tauntText(font, _taunt, MenuConfig::FONT_WIN_TAUNT);
    tauntText.setFillColor(sf::Color(170, 170, 170));
    tauntText.setStyle(sf::Text::Italic);
    centerAt(tauntText, height * MenuConfig::WIN_TAUNT_Y);
    window.draw(tauntText);

    sf::Text promptText(font, "PRESS ENTER", MenuConfig::FONT_WIN_PROMPT);
    promptText.setFillColor(sf::Color(130, 130, 130));
    centerAt(promptText, height * MenuConfig::WIN_PROMPT_Y);
    window.draw(promptText);
}
