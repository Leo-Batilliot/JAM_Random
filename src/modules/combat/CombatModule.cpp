#include "CombatModule.hpp"
#include "core/GameState.hpp"
#include "core/InputManager.hpp"
#include "config/ArenaConfig.hpp"
#include "config/ColorConfig.hpp"
#include "config/GameplayConfig.hpp"
#include "config/WindowConfig.hpp"
#include <SFML/Graphics/CircleShape.hpp>
#include <algorithm>

CombatModule::CombatModule(ModuleContext& context) : _context(context) {
	_facing[0] = {1.f, 0.f};
	_facing[1] = {-1.f, 0.f};
}

void CombatModule::handleEvent(const ModuleContext&, const sf::Event&) {}

void CombatModule::update(const ModuleContext& context, float dt) {
	constexpr sf::Vector2f arenaSize{
		WindowConfig::WINDOW_WIDTH * ArenaConfig::ARENA_WIDTH_RATIO,
		WindowConfig::WINDOW_HEIGHT * ArenaConfig::ARENA_HEIGHT_RATIO
	};
	constexpr sf::Vector2f arenaCenter{
		WindowConfig::WINDOW_WIDTH * ArenaConfig::ARENA_POS_X_RATIO,
		WindowConfig::WINDOW_HEIGHT * ArenaConfig::ARENA_POS_Y_RATIO
	};
	constexpr sf::Vector2f arenaTopLeft{
		arenaCenter.x - arenaSize.x * 0.5f,
		arenaCenter.y - arenaSize.y * 0.5f
	};

	for (int playerIndex = 0; playerIndex < GameplayConfig::PLAYER_COUNT; ++playerIndex) {
		PlayerState& player = context.state.players[playerIndex];
		if (!player.alive)
			continue;
		const auto&[moveDir, shoot] = context.input.get(playerIndex);
		player.position += moveDir * GameplayConfig::PLAYER_SPEED * dt;

		float minX = arenaTopLeft.x + player.radius;
		float maxX = arenaTopLeft.x + arenaSize.x - player.radius;
		float minY = arenaTopLeft.y + player.radius;
		float maxY = arenaTopLeft.y + arenaSize.y - player.radius;
		player.position.x = std::clamp(player.position.x, minX, maxX);
		player.position.y = std::clamp(player.position.y, minY, maxY);

		if (moveDir.x != 0.f || moveDir.y != 0.f)
			_facing[playerIndex] = moveDir;

		_fireCooldowns[playerIndex] = std::max(0.f, _fireCooldowns[playerIndex] - dt);

		if (shoot && _fireCooldowns[playerIndex] <= 0.f) {
			_bullets.push_back(Bullet{
				player.position,
				_facing[playerIndex] * GameplayConfig::BULLET_SPEED,
				playerIndex,
				GameplayConfig::BULLET_LIFETIME
			});
			_fireCooldowns[playerIndex] = GameplayConfig::FIRE_COOLDOWN;
		}
	}

	for (auto& bullet : _bullets) {
		bullet.position += bullet.velocity * dt;
		bullet.lifetime -= dt;
	}

	for (auto& bullet : _bullets) {
		if (bullet.lifetime <= 0.f)
			continue;
		for (int playerIndex = 0; playerIndex < GameplayConfig::PLAYER_COUNT; ++playerIndex) {
			if (playerIndex == bullet.ownerId)
				continue;
			PlayerState& player = context.state.players[playerIndex];
			if (!player.alive || player.invincible)
				continue;
			const sf::Vector2f delta = player.position - bullet.position;
			const float distSq = delta.x * delta.x + delta.y * delta.y;
			if (float radiusSum = player.radius + GameplayConfig::BULLET_RADIUS; distSq > radiusSum * radiusSum)
				continue;

			bullet.lifetime = -1.f;
			player.hp -= 1;
			if (player.hp > 0) {
				player.invincible = true;
				player.invincibleTimer = GameplayConfig::PLAYER_INVINCIBILITY_DURATION;
				context.bus.emit(PlayerHitEvent{playerIndex, bullet.ownerId});
			} else {
				player.alive = false;
				player.respawnTimer = GameplayConfig::PLAYER_RESPAWN_DELAY;
				int killerId = bullet.ownerId;
				PlayerState& killer = context.state.players[killerId];
				killer.kills += 1;
				context.bus.emit(PlayerDiedEvent{playerIndex, killerId, player.position});
				if (killer.kills >= GameplayConfig::KILLS_TO_WIN && !context.state.gameOver) {
					context.state.gameOver = true;
					context.state.winnerId = killerId;
					context.bus.emit(GameWonEvent{killerId, killer.kills});
				}
			}
			break;
		}
	}

	constexpr float arenaMinX = arenaTopLeft.x;
	constexpr float arenaMaxX = arenaTopLeft.x + arenaSize.x;
	constexpr float arenaMinY = arenaTopLeft.y;
	constexpr float arenaMaxY = arenaTopLeft.y + arenaSize.y;
	std::erase_if(_bullets, [&](const Bullet& b) {
		return b.lifetime <= 0.f
		       || b.position.x < arenaMinX || b.position.x > arenaMaxX
		       || b.position.y < arenaMinY || b.position.y > arenaMaxY;
	});

	for (int playerIndex = 0; playerIndex < GameplayConfig::PLAYER_COUNT; ++playerIndex) {
		if (PlayerState& player = context.state.players[playerIndex]; player.alive) {
			if (player.invincible) {
				player.invincibleTimer -= dt;
				if (player.invincibleTimer <= 0.f) {
					player.invincible = false;
					player.invincibleTimer = 0.f;
				}
			}
		} else {
			player.respawnTimer -= dt;
			if (player.respawnTimer <= 0.f) {
				player.alive = true;
				player.hp = GameplayConfig::PLAYER_MAX_HP;
				player.position = player.spawnPosition;
				player.invincible = true;
				player.invincibleTimer = GameplayConfig::PLAYER_INVINCIBILITY_DURATION;
				player.respawnTimer = 0.f;
				context.bus.emit(PlayerRespawnEvent{playerIndex, player.spawnPosition});
			}
		}
	}
}

void CombatModule::render(const ModuleContext& context, sf::RenderWindow& window) {
	for (int playerIndex = 0; playerIndex < GameplayConfig::PLAYER_COUNT; ++playerIndex) {
		const PlayerState& player = context.state.players[playerIndex];
		if (!player.alive)
			continue;
		sf::CircleShape body(player.radius);
		body.setOrigin(sf::Vector2f{player.radius, player.radius});
		body.setPosition(player.position);
		sf::Color color = (playerIndex == 0) ? ColorConfig::PLAYER0_COLOR : ColorConfig::PLAYER1_COLOR;
		if (player.invincible) {
			const int phase = static_cast<int>(player.invincibleTimer * 10.f) % 2;
			color.a = (phase == 0) ? 80 : 255;
		}
		body.setFillColor(color);
		window.draw(body);
	}

	for (const auto& bullet : _bullets) {
		sf::CircleShape shape(GameplayConfig::BULLET_RADIUS);
		shape.setOrigin(sf::Vector2f{GameplayConfig::BULLET_RADIUS, GameplayConfig::BULLET_RADIUS});
		shape.setPosition(bullet.position);
		const sf::Color& color = (bullet.ownerId == 0) ? ColorConfig::PLAYER0_COLOR : ColorConfig::PLAYER1_COLOR;
		shape.setFillColor(color);
		window.draw(shape);
	}
}
