#pragma once
#include "../core/Scene.hpp"
#include <string>

class Game;

class WinScene : public Scene {
public:
    WinScene(Game& game, int winnerId);
    void handleEvent(const sf::Event& event, SceneManager& sm) override;
    void update(float dt, SceneManager& sm) override;
    void render(sf::RenderWindow& window) override;

private:
    Game& _game;
    int _winnerId;
    std::string _taunt;
};
