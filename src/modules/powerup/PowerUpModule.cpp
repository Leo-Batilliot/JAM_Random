#include "PowerUpModule.hpp"
#include "config/ArenaConfig.hpp"
#include "config/GameplayConfig.hpp"
#include "config/PowerUpConfig.hpp"
#include "config/WindowConfig.hpp"
#include "config/WorldConfig.hpp"
#include "core/AssetManager.hpp"
#include "core/EventBus.hpp"
#include "core/GameState.hpp"
#include <SFML/Graphics/Sprite.hpp>
#include <algorithm>
#include <array>
#include <cmath>

PowerUpModule::PowerUpModule(ModuleContext& context)
	: _context(context), _rng(std::random_device{}()) {
	resetSpawnTimer();
}

void PowerUpModule::handleEvent(const ModuleContext&, const sf::Event&) {}

void PowerUpModule::update(const ModuleContext& context, float dt) {
	tickSpawn(context, dt);
	tickDespawn(dt);
	tickPickups(context);
	tickEffects(context, dt);
	enforceContinuousEffects(context);
}

void PowerUpModule::render(const ModuleContext&, sf::RenderWindow& window) {
	const float diameter = PowerUpConfig::POWERUP_RADIUS * 2.f;
	sf::CircleShape fallbackShape(PowerUpConfig::POWERUP_RADIUS);
	fallbackShape.setOrigin(sf::Vector2f{PowerUpConfig::POWERUP_RADIUS, PowerUpConfig::POWERUP_RADIUS});

	for (const MapPowerUp& powerUp : _powerUps) {
		const sf::Color tierColor = colorFromTier(powerUp.tier);
		try {
			const sf::Texture& texture = AssetManager::instance().texture(std::string(texturePathFromType(powerUp.type)));
			sf::Sprite sprite(texture);
			sprite.setOrigin(sf::Vector2f{texture.getSize().x * 0.5f, texture.getSize().y * 0.5f});
			sprite.setPosition(powerUp.position);
			sprite.setColor(tierColor);
			sprite.setScale(sf::Vector2f{
				diameter / static_cast<float>(texture.getSize().x),
				diameter / static_cast<float>(texture.getSize().y)
			});
			window.draw(sprite);
		} catch (...) {
			fallbackShape.setPosition(powerUp.position);
			fallbackShape.setFillColor(colorFromTier(powerUp.tier));
			fallbackShape.setOutlineColor(tintFromType(powerUp.type));
			fallbackShape.setOutlineThickness(2.f);
			window.draw(fallbackShape);
		}
	}
}

void PowerUpModule::resetSpawnTimer() {
	std::uniform_real_distribution<float> spawnDist(
		PowerUpConfig::MIN_SPAWN_INTERVAL,
		PowerUpConfig::MAX_SPAWN_INTERVAL
	);
	_spawnTimer = spawnDist(_rng);
}

sf::FloatRect PowerUpModule::arenaBounds() const {
	const sf::Vector2f arenaSize{
		WindowConfig::WINDOW_WIDTH * ArenaConfig::ARENA_WIDTH_RATIO,
		WindowConfig::WINDOW_HEIGHT * ArenaConfig::ARENA_HEIGHT_RATIO
	};
	const sf::Vector2f arenaCenter{
		WindowConfig::WINDOW_WIDTH * ArenaConfig::ARENA_POS_X_RATIO,
		WindowConfig::WINDOW_HEIGHT * ArenaConfig::ARENA_POS_Y_RATIO
	};
	return sf::FloatRect(
		sf::Vector2f{arenaCenter.x - arenaSize.x * 0.5f, arenaCenter.y - arenaSize.y * 0.5f},
		arenaSize
	);
}

void PowerUpModule::tickSpawn(const ModuleContext& context, float dt) {
	_spawnTimer -= dt;
	if (_spawnTimer > 0.f)
		return;

	resetSpawnTimer();
	if (static_cast<int>(_powerUps.size()) >= PowerUpConfig::POWERUP_MAX_ON_MAP)
		return;

	const sf::FloatRect bounds = arenaBounds();
	const float wallBoundary = WorldConfig::WALL_THICKNESS + WorldConfig::WALL_FACE_DEPTH;
	float maxPlayerRadius = GameplayConfig::PLAYER_RADIUS;
	for (const PlayerState& player : context.state.players)
		maxPlayerRadius = std::max(maxPlayerRadius, player.radius);

	const float spawnMargin = wallBoundary + maxPlayerRadius + PowerUpConfig::POWERUP_RADIUS;
	const float minX = bounds.position.x + spawnMargin;
	const float maxX = bounds.position.x + bounds.size.x - spawnMargin;
	const float minY = bounds.position.y + spawnMargin;
	const float maxY = bounds.position.y + bounds.size.y - spawnMargin;
	if (minX >= maxX || minY >= maxY)
		return;

	std::uniform_real_distribution<float> xDist(minX, maxX);
	std::uniform_real_distribution<float> yDist(minY, maxY);

	const sf::Vector2f position{xDist(_rng), yDist(_rng)};
	const int diceValue = rollDice(-1);
	const int tier = tierFromDice(diceValue);
	const PowerUpType type = randomPowerUpType();

	_powerUps.push_back(MapPowerUp{position, type, tier, PowerUpConfig::POWERUP_LIFETIME});
	context.bus.emit(PowerUpSpawnedEvent{position, type, tier});
}

void PowerUpModule::tickDespawn(float dt) {
	for (MapPowerUp& powerUp : _powerUps)
		powerUp.timeRemaining -= dt;

	_powerUps.erase(
		std::remove_if(
			_powerUps.begin(),
			_powerUps.end(),
			[](const MapPowerUp& powerUp) { return powerUp.timeRemaining <= 0.f; }
		),
		_powerUps.end()
	);
}

void PowerUpModule::tickPickups(const ModuleContext& context) {
	for (int playerId = 0; playerId < static_cast<int>(context.state.players.size()); ++playerId) {
		PlayerState& player = context.state.players[playerId];
		if (!player.alive)
			continue;

		for (int i = static_cast<int>(_powerUps.size()) - 1; i >= 0; --i) {
			const sf::Vector2f delta = player.position - _powerUps[i].position;
			const float collisionDistance = player.radius + PowerUpConfig::POWERUP_RADIUS;
			const float distanceSq = delta.x * delta.x + delta.y * delta.y;
			if (distanceSq > collisionDistance * collisionDistance)
				continue;

			const float duration = durationFromTier(_powerUps[i].tier);
			player.addEffect(_powerUps[i].type, duration);
			context.bus.emit(PowerUpPickedEvent{playerId, _powerUps[i].type, duration});
			_powerUps.erase(_powerUps.begin() + i);
		}
	}
}

void PowerUpModule::tickEffects(const ModuleContext& context, float dt) {
	for (int playerId = 0; playerId < static_cast<int>(context.state.players.size()); ++playerId)
		context.state.players[playerId].tickEffects(dt, context.bus, playerId);
}

void PowerUpModule::enforceContinuousEffects(const ModuleContext& context) {
	for (int playerId = 0; playerId < static_cast<int>(context.state.players.size()); ++playerId) {
		PlayerState& player = context.state.players[playerId];
		if (!player.alive)
			continue;

		const bool protectsPlayer =
			player.hasEffect(PowerUpType::Shield) || player.hasEffect(PowerUpType::Invincibility);
		if (!protectsPlayer)
			continue;

		player.invincible = true;
		player.invincibleTimer = std::max(player.invincibleTimer, PowerUpConfig::INVINCIBILITY_REFRESH_TIME);
	}
}

int PowerUpModule::rollDice(int playerId) {
	std::uniform_int_distribution<int> diceDist(1, 6);
	const int value = diceDist(_rng);
	_context.bus.emit(DiceRolledEvent{playerId, value});
	return value;
}

PowerUpType PowerUpModule::randomPowerUpType() {
	static constexpr std::array<PowerUpType, 7> types = {
		PowerUpType::SpeedBoost,
		PowerUpType::RapidFire,
		PowerUpType::SpreadShot,
		PowerUpType::Shield,
		PowerUpType::HomingBullets,
		PowerUpType::Invincibility,
		PowerUpType::TripleDamage
	};
	std::uniform_int_distribution<std::size_t> typeDist(0, types.size() - 1);
	return types[typeDist(_rng)];
}

int PowerUpModule::tierFromDice(int diceValue) const {
	if (diceValue >= 6)
		return 3;
	if (diceValue >= 4)
		return 2;
	return 1;
}

float PowerUpModule::durationFromTier(int tier) const {
	const std::size_t index = static_cast<std::size_t>(std::clamp(tier, 1, 3) - 1);
	return PowerUpConfig::POWERUP_EFFECT_DURATION * PowerUpConfig::TIER_DURATION_MULTIPLIERS[index];
}

sf::Color PowerUpModule::colorFromTier(int tier) const {
	const std::size_t index = static_cast<std::size_t>(std::clamp(tier, 1, 3) - 1);
	return PowerUpConfig::TIER_COLORS[index];
}

sf::Color PowerUpModule::tintFromType(PowerUpType type) const {
	const std::size_t index = static_cast<std::size_t>(type);
	if (index >= PowerUpConfig::TYPE_TINT_COLORS.size())
		return sf::Color::White;
	return PowerUpConfig::TYPE_TINT_COLORS[index];
}

std::string_view PowerUpModule::texturePathFromType(PowerUpType type) const {
	const std::size_t index = static_cast<std::size_t>(type);
	if (index >= PowerUpConfig::TYPE_ICON_TEXTURE_PATHS.size())
		return PowerUpConfig::TYPE_ICON_TEXTURE_PATHS[0];
	return PowerUpConfig::TYPE_ICON_TEXTURE_PATHS[index];
}
