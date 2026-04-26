#include "CombatModule.hpp"
#include "core/GameState.hpp"
#include "core/InputManager.hpp"
#include "config/ArenaConfig.hpp"
#include "config/ColorConfig.hpp"
#include "config/GameplayConfig.hpp"
#include "config/PowerUpConfig.hpp"
#include "config/WindowConfig.hpp"
#include "config/WorldConfig.hpp"
#include "config/VisualConfig.hpp"
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <limits>
#include <numbers>

namespace {

sf::Vector2f normalizeOrFallback(const sf::Vector2f& value, const sf::Vector2f& fallback) {
    const float len = std::sqrt(value.x * value.x + value.y * value.y);
    if (len <= 0.0001f)
        return fallback;
    return value / len;
}

sf::Vector2f rotateVector(const sf::Vector2f& vector, float degrees) {
    const float radians = degrees * (std::numbers::pi_v<float> / 180.f);
    const float cosA = std::cos(radians);
    const float sinA = std::sin(radians);
    return sf::Vector2f{
        vector.x * cosA - vector.y * sinA,
        vector.x * sinA + vector.y * cosA
    };
}

float speedMultiplier(const PlayerState& player) {
    return player.hasEffect(PowerUpType::SpeedBoost) ? PowerUpConfig::SPEED_BOOST_MULTIPLIER : 1.f;
}

float cooldownMultiplier(const PlayerState& player) {
    return player.hasEffect(PowerUpType::RapidFire) ? PowerUpConfig::RAPID_FIRE_COOLDOWN_MULTIPLIER : 1.f;
}

int bulletDamage(const PlayerState& player) {
    return player.hasEffect(PowerUpType::TripleDamage)
        ? PowerUpConfig::TRIPLE_DAMAGE_BULLET_DAMAGE
        : PowerUpConfig::NORMAL_BULLET_DAMAGE;
}

bool hasHoming(const PlayerState& player) {
    return player.hasEffect(PowerUpType::HomingBullets);
}

bool hasSpread(const PlayerState& player) {
    return player.hasEffect(PowerUpType::SpreadShot);
}

bool hasDefenseEffect(const PlayerState& player) {
    return player.hasEffect(PowerUpType::Shield) || player.hasEffect(PowerUpType::Invincibility);
}

} // namespace

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

        player.position += moveDirection * GameplayConfig::PLAYER_SPEED * speedMultiplier(player) * dt;
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
            const sf::Vector2f shootDirection = normalizeOrFallback(_facing[playerIndex], sf::Vector2f{1.f, 0.f});
            const int projectileCount = hasSpread(player) ? PowerUpConfig::SPREAD_PROJECTILE_COUNT : 1;
            const float spreadAngleStep = (projectileCount > 1) ? PowerUpConfig::SPREAD_ANGLE_DEGREES : 0.f;
            const float spreadStartAngle = -0.5f * spreadAngleStep * static_cast<float>(projectileCount - 1);

            for (int projectileIndex = 0; projectileIndex < projectileCount; ++projectileIndex) {
                const float angleOffset = spreadStartAngle + spreadAngleStep * static_cast<float>(projectileIndex);
                const sf::Vector2f shotDirection = (projectileCount > 1)
                    ? rotateVector(shootDirection, angleOffset)
                    : shootDirection;

                _bullets.push_back(Bullet{
                    {player.position.x + GameplayConfig::BULLET_SPAWN_OFFSET_X,
                     player.position.y + GameplayConfig::BULLET_SPAWN_OFFSET_Y},
                    normalizeOrFallback(shotDirection, shootDirection) * GameplayConfig::BULLET_SPEED,
                    playerIndex,
                    GameplayConfig::BULLET_LIFETIME,
                    bulletDamage(player),
                    hasHoming(player)
                });
            }

            const float cooldown = GameplayConfig::FIRE_COOLDOWN * cooldownMultiplier(player);
            _fireCooldowns[playerIndex] = std::max(cooldown, PowerUpConfig::MIN_FIRE_COOLDOWN);
        }
    }

    // Bullet movement
    for (auto& bullet : _bullets) {
        if (bullet.homing) {
            int targetId = -1;
            float closestDistanceSq = std::numeric_limits<float>::max();
            for (int playerIndex = 0; playerIndex < GameplayConfig::PLAYER_COUNT; ++playerIndex) {
                if (playerIndex == bullet.ownerId)
                    continue;
                const PlayerState& target = context.state.players[playerIndex];
                if (!target.alive)
                    continue;
                const sf::Vector2f toTarget = target.position - bullet.position;
                const float distanceSq = toTarget.x * toTarget.x + toTarget.y * toTarget.y;
                if (distanceSq < closestDistanceSq) {
                    closestDistanceSq = distanceSq;
                    targetId = playerIndex;
                }
            }

            if (targetId >= 0) {
                const sf::Vector2f toTarget = context.state.players[targetId].position - bullet.position;
                const sf::Vector2f desiredDir = normalizeOrFallback(toTarget, normalizeOrFallback(bullet.velocity, sf::Vector2f{1.f, 0.f}));
                const sf::Vector2f currentDir = normalizeOrFallback(bullet.velocity, desiredDir);
                const float steer = std::clamp(PowerUpConfig::HOMING_TURN_RATE * dt, 0.f, 1.f);
                const sf::Vector2f blendedDir = normalizeOrFallback(
                    currentDir * (1.f - steer) + desiredDir * steer,
                    desiredDir
                );
                bullet.velocity = blendedDir * GameplayConfig::BULLET_SPEED;
            }
        }

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
            if (!player.alive || player.invincible || hasDefenseEffect(player))
                continue;

            const sf::Vector2f delta = player.position - bullet.position;
            const float distanceSquared = delta.x * delta.x + delta.y * delta.y;
            const float radiusSum = player.radius + GameplayConfig::BULLET_RADIUS;
            if (distanceSquared > radiusSum * radiusSum)
                continue;

            bullet.lifetime = -1.f;
            player.hp -= bullet.damage;
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
