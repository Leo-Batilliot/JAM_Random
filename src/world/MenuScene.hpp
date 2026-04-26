#pragma once
#include "../core/Scene.hpp"

class Game;

class MenuScene : public Scene {
public:
    explicit MenuScene(Game& game);
    void handleEvent(const sf::Event& event, SceneManager& sm) override;
    void update(float dt, SceneManager& sm) override;
    void render(sf::RenderWindow& window) override;

private:
    Game& _game;
    float _blinkTimer = 0.f;
    bool _promptVisible = true;
};
