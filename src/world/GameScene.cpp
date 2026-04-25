#include "GameScene.hpp"
#include "WinScene.hpp"
#include "../core/Game.hpp"
#include "config/ArenaConfig.hpp"
#include "config/AssetConfig.hpp"
#include "config/ColorConfig.hpp"
#include "config/DecorConfig.hpp"
#include "config/WindowConfig.hpp"
#include "config/WorldConfig.hpp"
#include "modules/ModuleFactory.hpp"
#include <SFML/Graphics.hpp>
#include <array>
#include <cmath>
#include <random>
#include <stdexcept>
#include <vector>

namespace {

struct RockDecoration {
    sf::Vector2f position;
    float scale;
    float collisionRadius;
    int textureIndex;
};

// Randomly places large and small rocks in the playable area
std::vector<RockDecoration> generateRocks(sf::Vector2f arenaTopLeft, sf::Vector2f arenaSize) {
    // mieux que rand
    std::mt19937 rng(std::random_device{}());

    const float boundary = WorldConfig::WALL_THICKNESS + WorldConfig::WALL_FACE_DEPTH;
    const float spawnMinX = arenaTopLeft.x + boundary + DecorConfig::ROCK_SPAWN_MARGIN;
    const float spawnMaxX = arenaTopLeft.x + arenaSize.x - boundary - DecorConfig::ROCK_SPAWN_MARGIN;
    const float spawnMinY = arenaTopLeft.y + boundary + DecorConfig::ROCK_SPAWN_MARGIN;
    const float spawnMaxY = arenaTopLeft.y + arenaSize.y - boundary - DecorConfig::ROCK_SPAWN_MARGIN;

    std::vector<RockDecoration> rocks;

    auto canPlace = [&](sf::Vector2f candidate, float radius) {
        for (const auto& existing : rocks) {
            const sf::Vector2f delta = candidate - existing.position;
            const float distance = std::sqrt(delta.x * delta.x + delta.y * delta.y);
            if (distance < radius + existing.collisionRadius + DecorConfig::ROCK_MIN_GAP)
                return false;
        }
        return true;
    };

    auto tryPlace = [&](float collisionRadius, float scale) {
        std::uniform_real_distribution<float> xDist(spawnMinX + collisionRadius, spawnMaxX - collisionRadius);
        std::uniform_real_distribution<float> yDist(spawnMinY + collisionRadius, spawnMaxY - collisionRadius);
        std::uniform_int_distribution<int> textureDist(0, 2);
        for (int attempt = 0; attempt < DecorConfig::ROCK_PLACE_ATTEMPTS; ++attempt) {
            const sf::Vector2f candidate = {xDist(rng), yDist(rng)};
            if (canPlace(candidate, collisionRadius)) {
                rocks.push_back({candidate, scale, collisionRadius, textureDist(rng)});
                return;
            }
        }
    };

    const int largeCount = std::uniform_int_distribution<int>(DecorConfig::ROCK_LARGE_COUNT_MIN, DecorConfig::ROCK_LARGE_COUNT_MAX)(rng);
    for (int i = 0; i < largeCount; ++i)
        tryPlace(DecorConfig::ROCK_LARGE_COLLISION_RADIUS, DecorConfig::ROCK_LARGE_SCALE);

    const int smallCount = std::uniform_int_distribution<int>(DecorConfig::ROCK_SMALL_COUNT_MIN, DecorConfig::ROCK_SMALL_COUNT_MAX)(rng);
    for (int i = 0; i < smallCount; ++i)
        tryPlace(DecorConfig::ROCK_SMALL_COLLISION_RADIUS, DecorConfig::ROCK_SMALL_SCALE);

    return rocks;
}

} // namespace

struct GameScene::SceneState {
    ModuleContext context;
    std::vector<std::unique_ptr<IGameModule>> modules;

    sf::Vector2f arenaTopLeft;
    sf::Vector2f arenaSize;
    sf::Texture floorTexture;
    sf::Texture wallTexture;
    std::array<sf::Texture, 3> rockTextures;
    std::vector<RockDecoration> rocks;

    explicit SceneState(Game& game)
        : context{game, game.bus(), game.state(), game.input()},
          modules(createGameModules(context))
    {
        arenaSize = {
            WindowConfig::WINDOW_WIDTH * ArenaConfig::ARENA_WIDTH_RATIO,
            WindowConfig::WINDOW_HEIGHT * ArenaConfig::ARENA_HEIGHT_RATIO
        };
        const sf::Vector2f arenaCenter = {
            WindowConfig::WINDOW_WIDTH * ArenaConfig::ARENA_POS_X_RATIO,
            WindowConfig::WINDOW_HEIGHT * ArenaConfig::ARENA_POS_Y_RATIO
        };
        arenaTopLeft = {arenaCenter.x - arenaSize.x * 0.5f, arenaCenter.y - arenaSize.y * 0.5f};

        if (!floorTexture.loadFromFile(AssetConfig::FLOOR_TEXTURE))
            throw std::runtime_error("cannot load floor texture");
        floorTexture.setRepeated(true);

        if (!wallTexture.loadFromFile(AssetConfig::WALL_TEXTURE))
            throw std::runtime_error("cannot load wall texture");
        wallTexture.setRepeated(true);

        if (!rockTextures[0].loadFromFile(AssetConfig::ROCK_1) ||
            !rockTextures[1].loadFromFile(AssetConfig::ROCK_2) ||
            !rockTextures[2].loadFromFile(AssetConfig::ROCK_3))
            throw std::runtime_error("cannot load rock textures");

        rocks = generateRocks(arenaTopLeft, arenaSize);
    }
};

GameScene::GameScene(Game& game)
    : _game(game), _sceneState(std::make_unique<SceneState>(game))
{
    const sf::Vector2f& topLeft = _sceneState->arenaTopLeft;
    const sf::Vector2f& arenaSize = _sceneState->arenaSize;
    _game.state().reset(
        {topLeft.x + arenaSize.x * ArenaConfig::ARENA_SPAWN0_X_RATIO,
         topLeft.y + arenaSize.y * ArenaConfig::ARENA_SPAWN_Y_RATIO},
        {topLeft.x + arenaSize.x * ArenaConfig::ARENA_SPAWN1_X_RATIO,
         topLeft.y + arenaSize.y * ArenaConfig::ARENA_SPAWN_Y_RATIO}
    );

    _game.bus().subscribe<GameWonEvent>([this](const GameWonEvent& event) {
        _game.sceneManager().replace(std::make_unique<WinScene>(_game, event.winnerId));
    });
}

// important (caused crashes when starting a second game)
GameScene::~GameScene() {
    _game.bus().clear();
}

void GameScene::handleEvent(const sf::Event& event, SceneManager& sceneManager) {
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (key->code == sf::Keyboard::Key::Escape) {
            sceneManager.pop();
            return;
        }
    }
    for (auto& module : _sceneState->modules)
        module->handleEvent(_sceneState->context, event);
}

void GameScene::update(float dt, SceneManager&) {
    for (auto& module : _sceneState->modules)
        module->update(_sceneState->context, dt);
}

void GameScene::render(sf::RenderWindow& window) {
    const float windowWidth = static_cast<float>(WindowConfig::WINDOW_WIDTH);
    const float windowHeight = static_cast<float>(WindowConfig::WINDOW_HEIGHT);
    const float wallThickness = WorldConfig::WALL_THICKNESS;
    const float wallFaceDepth = WorldConfig::WALL_FACE_DEPTH;
    const sf::Vector2f& topLeft = _sceneState->arenaTopLeft;
    const sf::Vector2f& arenaSize = _sceneState->arenaSize;

    // Background
    sf::RectangleShape background({windowWidth, windowHeight});
    background.setFillColor(ColorConfig::BACKGROUND_COLOR);
    window.draw(background);

    // Floor
    {
        const auto textureSize = _sceneState->floorTexture.getSize();
        sf::RectangleShape floor(arenaSize);
        floor.setPosition(topLeft);
        floor.setTexture(&_sceneState->floorTexture);
        floor.setTextureRect(sf::IntRect({0, 0}, {
            (int)(arenaSize.x * (float)textureSize.x / DecorConfig::FLOOR_TILE_SIZE),
            (int)(arenaSize.y * (float)textureSize.y / DecorConfig::FLOOR_TILE_SIZE)
        }));
        floor.setFillColor(ColorConfig::FLOOR_TINT);
        window.draw(floor);
    }

    // Rocks
    for (const auto& rock : _sceneState->rocks) {
        const auto& rockTexture = _sceneState->rockTextures[rock.textureIndex];
        const auto rockTextureSize = rockTexture.getSize();
        sf::Sprite rockSprite(rockTexture);
        rockSprite.setOrigin({rockTextureSize.x * 0.5f, rockTextureSize.y * 0.5f});
        rockSprite.setScale({rock.scale, rock.scale});
        rockSprite.setPosition(rock.position);
        window.draw(rockSprite);
    }

    // Wall faces
    {
        const auto wallTextureSize = _sceneState->wallTexture.getSize();

        auto drawHorizontalFace = [&](sf::Vector2f position, float width, float height) {
            const float uRange = height * (float)wallTextureSize.y / DecorConfig::WALL_TILE_SIZE;
            const float vRange = width  * (float)wallTextureSize.x / DecorConfig::WALL_TILE_SIZE;
            sf::VertexArray quad(sf::PrimitiveType::TriangleStrip, 4);
            quad[0].position  = position;
            quad[1].position  = {position.x + width, position.y};
            quad[2].position  = {position.x, position.y + height};
            quad[3].position  = {position.x + width, position.y + height};
            quad[0].texCoords = {0.f, 0.f};
            quad[1].texCoords = {0.f, vRange};
            quad[2].texCoords = {uRange, 0.f};
            quad[3].texCoords = {uRange, vRange};
            for (int i = 0; i < 4; ++i)
                quad[i].color = ColorConfig::WALL_FACE_COLOR;
            sf::RenderStates renderStates;
            renderStates.texture = &_sceneState->wallTexture;
            window.draw(quad, renderStates);
        };

        auto drawVerticalFace = [&](sf::Vector2f position, float width, float height) {
            const float uRange = width  * (float)wallTextureSize.x / DecorConfig::WALL_TILE_SIZE;
            const float vRange = height * (float)wallTextureSize.y / DecorConfig::WALL_TILE_SIZE;
            sf::VertexArray quad(sf::PrimitiveType::TriangleStrip, 4);
            quad[0].position  = position;
            quad[1].position  = {position.x + width, position.y};
            quad[2].position  = {position.x, position.y + height};
            quad[3].position  = {position.x + width, position.y + height};
            quad[0].texCoords = {0.f, 0.f};
            quad[1].texCoords = {uRange, 0.f};
            quad[2].texCoords = {0.f, vRange};
            quad[3].texCoords = {uRange, vRange};
            for (int i = 0; i < 4; ++i)
                quad[i].color = ColorConfig::WALL_FACE_COLOR;
            sf::RenderStates renderStates;
            renderStates.texture = &_sceneState->wallTexture;
            window.draw(quad, renderStates);
        };

        drawHorizontalFace({topLeft.x, topLeft.y + wallThickness}, arenaSize.x, wallFaceDepth);
        drawHorizontalFace({topLeft.x, topLeft.y + arenaSize.y - wallThickness - wallFaceDepth},
                           arenaSize.x, wallFaceDepth);
        drawVerticalFace({topLeft.x + wallThickness, topLeft.y + wallThickness + wallFaceDepth},
                         wallFaceDepth, arenaSize.y - 2.f * wallThickness - 2.f * wallFaceDepth);
        drawVerticalFace({topLeft.x + arenaSize.x - wallThickness - wallFaceDepth,
                          topLeft.y + wallThickness + wallFaceDepth},
                         wallFaceDepth, arenaSize.y - 2.f * wallThickness - 2.f * wallFaceDepth);
    }

    // Wall surfaces
    {
        const auto wallTextureSize = _sceneState->wallTexture.getSize();
        auto drawWallSurface = [&](sf::Vector2f position, sf::Vector2f size) {
            sf::RectangleShape wall(size);
            wall.setPosition(position);
            wall.setTexture(&_sceneState->wallTexture);
            wall.setTextureRect(sf::IntRect({0, 0}, {
                (int)(size.x * (float)wallTextureSize.x / DecorConfig::WALL_TILE_SIZE),
                (int)(size.y * (float)wallTextureSize.y / DecorConfig::WALL_TILE_SIZE)
            }));
            window.draw(wall);
        };
        drawWallSurface({topLeft.x, topLeft.y}, {arenaSize.x, wallThickness});
        drawWallSurface({topLeft.x, topLeft.y + arenaSize.y - wallThickness}, {arenaSize.x, wallThickness});
        drawWallSurface({topLeft.x, topLeft.y + wallThickness}, {wallThickness, arenaSize.y - 2.f * wallThickness});
        drawWallSurface({topLeft.x + arenaSize.x - wallThickness, topLeft.y + wallThickness},
                        {wallThickness, arenaSize.y - 2.f * wallThickness});
    }

    // Modules
    for (auto& module : _sceneState->modules)
        module->render(_sceneState->context, window);
}
