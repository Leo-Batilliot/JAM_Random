#pragma once
#include "../IGameModule.hpp"
#include "config/GameplayConfig.hpp"
#include <SFML/System/Vector2.hpp>
#include <array>
#include <vector>

// Module A: combat, gameplay rules for players, bullets, and kills.
class CombatModule : public IGameModule {
public:
    explicit CombatModule(ModuleContext& context);
    void handleEvent(const ModuleContext& context, const sf::Event& event) override;
    void update(const ModuleContext& context, float dt) override;
    void render(const ModuleContext& context, sf::RenderWindow& window) override;

private:
    struct Bullet {
        sf::Vector2f position;
        sf::Vector2f velocity;
        int ownerId;
        float lifetime;
        int damage;
        bool homing;
    };

    ModuleContext& _context;
    std::vector<Bullet> _bullets;
    std::array<float, GameplayConfig::PLAYER_COUNT> _fireCooldowns{};
    std::array<sf::Vector2f, GameplayConfig::PLAYER_COUNT> _facing{};
};
