#pragma once
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

class SceneManager;

class Scene {
public:
    virtual ~Scene() = default;
    virtual void handleEvent(const sf::Event& event, SceneManager& sm) = 0;
    virtual void update(float dt, SceneManager& sm) = 0;
    virtual void render(sf::RenderWindow& window) = 0;
    virtual void onResume() {}
};
