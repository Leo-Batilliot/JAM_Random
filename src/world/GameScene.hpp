#pragma once
#include "../core/Scene.hpp"
#include <memory>

class Game;

class GameScene : public Scene {
public:
    explicit GameScene(Game& game);
    ~GameScene() override;
    void handleEvent(const sf::Event& event, SceneManager& sm) override;
    void update(float dt, SceneManager& sm) override;
    void render(sf::RenderWindow& window) override;

private:
    Game& _game;
    struct SceneState;
    std::unique_ptr<SceneState> _sceneState;
};
