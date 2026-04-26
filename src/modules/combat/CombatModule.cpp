#include "CombatModule.hpp"
#include "core/GameState.hpp"
#include "core/InputManager.hpp"
#include "config/ArenaConfig.hpp"
#include "config/ColorConfig.hpp"
#include "config/GameplayConfig.hpp"
#include "config/WindowConfig.hpp"
#include "config/WorldConfig.hpp"
#include "config/VisualConfig.hpp"
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>

CombatModule::CombatModule(ModuleContext& context) : _context(context) {
    _facing[0] = {1.f, 0.f};
    _facing[1] = {-1.f, 0.f};
}

void CombatModule::handleEvent(const ModuleContext&, const sf::Event&) {}

void CombatModule::update(const ModuleContext& context, float dt) {
    // Arena bounds
    const sf::Vector2f arenaSize{
        WindowConfig::WINDOW_WIDTH * ArenaConfig::ARENA_WIDTH_RATIO,
        WindowConfig::WINDOW_HEIGHT * ArenaConfig::ARENA_HEIGHT_RATIO
    };
    const sf::Vector2f arenaCenter{
        WindowConfig::WINDOW_WIDTH * ArenaConfig::ARENA_POS_X_RATIO,
        WindowConfig::WINDOW_HEIGHT * ArenaConfig::ARENA_POS_Y_RATIO
    };
    const sf::Vector2f arenaTopLeft{
        arenaCenter.x - arenaSize.x * 0.5f,
        arenaCenter.y - arenaSize.y * 0.5f
    };
    const float wallBoundary = WorldConfig::WALL_THICKNESS + WorldConfig::WALL_FACE_DEPTH;

    // Player movement + shooting
    for (int playerIndex = 0; playerIndex < GameplayConfig::PLAYER_COUNT; ++playerIndex) {
        PlayerState& player = context.state.players[playerIndex];
        if (!player.alive)
            continue;

        const PlayerInput& playerInput = context.input.get(playerIndex);
        const sf::Vector2f moveDirection = playerInput.moveDir;
        const bool wantsToShoot = playerInput.shoot;

        player.position += moveDirection * GameplayConfig::PLAYER_SPEED * dt;
        player.position.x = std::clamp(player.position.x,
            arenaTopLeft.x + wallBoundary + player.radius,
            arenaTopLeft.x + arenaSize.x - wallBoundary - player.radius);
        player.position.y = std::clamp(player.position.y,
            arenaTopLeft.y + wallBoundary + player.radius,
            arenaTopLeft.y + arenaSize.y - wallBoundary - player.radius);

        if (moveDirection.x != 0.f || moveDirection.y != 0.f)
            _facing[playerIndex] = moveDirection;

        _fireCooldowns[playerIndex] = std::max(0.f, _fireCooldowns[playerIndex] - dt);

        if (wantsToShoot && _fireCooldowns[playerIndex] <= 0.f) {
            _bullets.push_back(Bullet{
                {player.position.x + GameplayConfig::BULLET_SPAWN_OFFSET_X,
                 player.position.y + GameplayConfig::BULLET_SPAWN_OFFSET_Y},
                _facing[playerIndex] * GameplayConfig::BULLET_SPEED,
                playerIndex,
                GameplayConfig::BULLET_LIFETIME
            });
            _fireCooldowns[playerIndex] = GameplayConfig::FIRE_COOLDOWN;
        }
    }

    // Bullet movement
    for (auto& bullet : _bullets) {
        bullet.position += bullet.velocity * dt;
        bullet.lifetime -= dt;
    }

    // Bullet collisions
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
            const float distanceSquared = delta.x * delta.x + delta.y * delta.y;
            const float radiusSum = player.radius + GameplayConfig::BULLET_RADIUS;
            if (distanceSquared > radiusSum * radiusSum)
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
                PlayerState& killer = context.state.players[bullet.ownerId];
                killer.kills += 1;
                context.bus.emit(PlayerDiedEvent{playerIndex, bullet.ownerId, player.position});
                if (killer.kills >= GameplayConfig::KILLS_TO_WIN && !context.state.gameOver) {
                    context.state.gameOver = true;
                    context.state.winnerId = bullet.ownerId;
                    context.bus.emit(GameWonEvent{bullet.ownerId, killer.kills});
                }
            }
            break;
        }
    }

    // Bullet cleanup
    const float playableLeft = arenaTopLeft.x + wallBoundary;
    const float playableRight = arenaTopLeft.x + arenaSize.x - wallBoundary;
    const float playableTop = arenaTopLeft.y + wallBoundary;
    const float playableBottom = arenaTopLeft.y + arenaSize.y - wallBoundary;

    std::erase_if(_bullets, [&](const Bullet& bullet) {
        return bullet.lifetime <= 0.f || bullet.position.x < playableLeft  || bullet.position.x > playableRight
            || bullet.position.y < playableTop || bullet.position.y > playableBottom;
    });

    // Respawn + invincibility timers
    for (int playerIndex = 0; playerIndex < GameplayConfig::PLAYER_COUNT; ++playerIndex) {
        PlayerState& player = context.state.players[playerIndex];
        if (player.alive) {
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
    // Player hitboxes
    for (int playerIndex = 0; playerIndex < GameplayConfig::PLAYER_COUNT; ++playerIndex) {
        const PlayerState& player = context.state.players[playerIndex];
        if (!player.alive)
            continue;

        sf::CircleShape circle(player.radius);
        circle.setOrigin({player.radius, player.radius});
        circle.setPosition(player.position);
        sf::Color playerColor = (playerIndex == 0) ? ColorConfig::PLAYER0_COLOR : ColorConfig::PLAYER1_COLOR;
        if (player.invincible) {
            const int blinkPhase = static_cast<int>(player.invincibleTimer * 10.f) % 2;
            playerColor.a = (blinkPhase == 0) ? VisualConfig::INVINCIBLE_BLINK_LOW_ALPHA : 255;
        }
        circle.setFillColor(playerColor);
        window.draw(circle);
    }

    // Bullets
    for (const auto& bullet : _bullets) {
        sf::CircleShape bulletShape(GameplayConfig::BULLET_RADIUS);
        bulletShape.setOrigin({GameplayConfig::BULLET_RADIUS, GameplayConfig::BULLET_RADIUS});
        bulletShape.setPosition(bullet.position);
        const bool isPlayer0 = bullet.ownerId == 0;
        bulletShape.setFillColor(isPlayer0 ? ColorConfig::BULLET_P0_FILL : ColorConfig::BULLET_P1_FILL);
        bulletShape.setOutlineColor(isPlayer0 ? ColorConfig::BULLET_P0_OUTLINE : ColorConfig::BULLET_P1_OUTLINE);
        bulletShape.setOutlineThickness(VisualConfig::BULLET_OUTLINE_THICKNESS);
        window.draw(bulletShape);
    }
}
