#include "CombatModule.hpp"
#include "core/GameState.hpp"
#include "core/InputManager.hpp"
#include "config/ArenaConfig.hpp"
#include "config/ColorConfig.hpp"
#include "config/GameplayConfig.hpp"
#include "config/WindowConfig.hpp"
#include <SFML/Graphics/CircleShape.hpp>
#include <algorithm>

CombatModule::CombatModule(ModuleContext& context) : _context(context) {}

void CombatModule::handleEvent(const ModuleContext&, const sf::Event&) {}

void CombatModule::update(const ModuleContext& context, float dt) {
	sf::Vector2f arenaSize{
		WindowConfig::WINDOW_WIDTH * ArenaConfig::ARENA_WIDTH_RATIO,
		WindowConfig::WINDOW_HEIGHT * ArenaConfig::ARENA_HEIGHT_RATIO
	};
	sf::Vector2f arenaCenter{
		WindowConfig::WINDOW_WIDTH * ArenaConfig::ARENA_POS_X_RATIO,
		WindowConfig::WINDOW_HEIGHT * ArenaConfig::ARENA_POS_Y_RATIO
	};
	sf::Vector2f arenaTopLeft{
		arenaCenter.x - arenaSize.x * 0.5f,
		arenaCenter.y - arenaSize.y * 0.5f
	};

	for (int playerIndex = 0; playerIndex < GameplayConfig::PLAYER_COUNT; ++playerIndex) {
		PlayerState& player = context.state.players[playerIndex];
		if (!player.alive)
			continue;
		const PlayerInput& input = context.input.get(playerIndex);
		player.position += input.moveDir * GameplayConfig::PLAYER_SPEED * dt;

		float minX = arenaTopLeft.x + player.radius;
		float maxX = arenaTopLeft.x + arenaSize.x - player.radius;
		float minY = arenaTopLeft.y + player.radius;
		float maxY = arenaTopLeft.y + arenaSize.y - player.radius;
		player.position.x = std::clamp(player.position.x, minX, maxX);
		player.position.y = std::clamp(player.position.y, minY, maxY);
	}
}

void CombatModule::render(const ModuleContext& context, sf::RenderWindow& window) {
	for (int playerIndex = 0; playerIndex < GameplayConfig::PLAYER_COUNT; ++playerIndex) {
		const PlayerState& player = context.state.players[playerIndex];
		sf::CircleShape body(player.radius);
		body.setOrigin(sf::Vector2f{player.radius, player.radius});
		body.setPosition(player.position);
		const sf::Color& color = (playerIndex == 0) ? ColorConfig::PLAYER0_COLOR : ColorConfig::PLAYER1_COLOR;
		body.setFillColor(color);
		window.draw(body);
	}
}
