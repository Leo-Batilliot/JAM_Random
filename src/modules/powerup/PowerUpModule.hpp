#pragma once
#include "../IGameModule.hpp"
#include "core/GameEvents.hpp"
#include <SFML/Graphics/CircleShape.hpp>
#include <string_view>
#include <random>
#include <vector>

// Module B: powerups, spawn logic and active effects.
class PowerUpModule : public IGameModule {
public:
    explicit PowerUpModule(ModuleContext& context);
    void handleEvent(const ModuleContext& context, const sf::Event& event) override;
    void update(const ModuleContext& context, float dt) override;
    void render(const ModuleContext& context, sf::RenderWindow& window) override;

private:
    struct MapPowerUp {
        sf::Vector2f position;
        PowerUpType type;
        int tier;
        float timeRemaining;
    };

    ModuleContext& _context;
    std::vector<MapPowerUp> _powerUps;
    std::mt19937 _rng;
    float _spawnTimer = 0.f;

    void resetSpawnTimer();
    sf::FloatRect arenaBounds() const;
    void tickSpawn(const ModuleContext& context, float dt);
    void tickDespawn(float dt);
    void tickPickups(const ModuleContext& context);
    void tickEffects(const ModuleContext& context, float dt);
    void enforceContinuousEffects(const ModuleContext& context);
    int rollDice(int playerId);
    PowerUpType randomPowerUpType();
    int tierFromDice(int diceValue) const;
    float durationFromTier(int tier) const;
    sf::Color colorFromTier(int tier) const;
    sf::Color tintFromType(PowerUpType type) const;
    std::string_view texturePathFromType(PowerUpType type) const;
};
