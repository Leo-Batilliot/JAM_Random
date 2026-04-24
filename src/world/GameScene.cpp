#include "GameScene.hpp"
#include "WinScene.hpp"
#include "../core/Game.hpp"
#include "config/ArenaConfig.hpp"
#include "config/ColorConfig.hpp"
#include "config/WindowConfig.hpp"
#include <SFML/Graphics.hpp>

// GameData = private scene data stored in the .cpp.
// Use it to keep module state here without exposing it in the header.
// Example :
// struct GameScene::GameData {
//     sf::RectangleShape arena;
//     float flashTimer = 0.f;
//     // std::unique_ptr<CombatManager> combat;
//     // std::unique_ptr<PowerUpManager> powerups;
//     // std::unique_ptr<HUD> hud;
// };
// In constructor: _data->arena.setSize({arenaSizeX, arenaSizeY});
// In update: _data->flashTimer = std::max(0.0f, _data->flashTimer - dt);
// In render: window.draw(_data->arena);
struct GameScene::GameData {};

GameScene::GameScene(Game& game)
    : _game(game), _data(std::make_unique<GameData>())
{
    sf::Vector2f arenaSize{
        WindowConfig::WINDOW_WIDTH * ArenaConfig::ARENA_WIDTH_RATIO,
        WindowConfig::WINDOW_HEIGHT * ArenaConfig::ARENA_HEIGHT_RATIO
    };
    sf::Vector2f arenaPos{
        WindowConfig::WINDOW_WIDTH * ArenaConfig::ARENA_POS_X_RATIO,
        WindowConfig::WINDOW_HEIGHT * ArenaConfig::ARENA_POS_Y_RATIO
    };
    sf::Vector2f spawn0{
        arenaPos.x + arenaSize.x * ArenaConfig::ARENA_SPAWN0_X_RATIO,
        arenaPos.y + arenaSize.y * ArenaConfig::ARENA_SPAWN_Y_RATIO
    };
    sf::Vector2f spawn1{
        arenaPos.x + arenaSize.x * ArenaConfig::ARENA_SPAWN1_X_RATIO,
        arenaPos.y + arenaSize.y * ArenaConfig::ARENA_SPAWN_Y_RATIO
    };
    _game.state().reset(spawn0, spawn1);

    _game.bus().subscribe<GameWonEvent>([this](const GameWonEvent& event) {
        _game.sceneManager().replace(std::make_unique<WinScene>(_game, event.winnerId));
    });
}

GameScene::~GameScene() = default;

void GameScene::handleEvent(const sf::Event& event, SceneManager& sm) {
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (key->code == sf::Keyboard::Key::Escape)
            sm.pop();
    }
    // Hook input/events here for gameplay systems if needed.
}

void GameScene::update(float, SceneManager&) {
    // Hook gameplay updates here (combat, powerups, HUD timers).
}

void GameScene::render(sf::RenderWindow& window) {
    // Hook renders here (arena first, then combat, powerups, HUD).
    sf::Vector2f arenaSize{
        WindowConfig::WINDOW_WIDTH * ArenaConfig::ARENA_WIDTH_RATIO,
        WindowConfig::WINDOW_HEIGHT * ArenaConfig::ARENA_HEIGHT_RATIO
    };
    sf::Vector2f arenaPos{
        WindowConfig::WINDOW_WIDTH * ArenaConfig::ARENA_POS_X_RATIO,
        WindowConfig::WINDOW_HEIGHT * ArenaConfig::ARENA_POS_Y_RATIO
    };
    sf::RectangleShape arena(arenaSize);
    arena.setOrigin(sf::Vector2f(arena.getSize().x / 2.f, arena.getSize().y / 2.f));
    arena.setPosition(arenaPos);
    arena.setFillColor(ColorConfig::ARENA_BACKGROUND_COLOR);
    arena.setOutlineColor(ColorConfig::ARENA_OUTLINE_COLOR);
    arena.setOutlineThickness(ArenaConfig::ARENA_OUTLINE_THICKNESS);
    window.draw(arena);
}
