#pragma once
#include "ModuleContext.hpp"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

// Base interface for gameplay modules
class IGameModule {
public:
    virtual ~IGameModule() = default;
    virtual void handleEvent(const ModuleContext& context, const sf::Event& event) = 0;
    virtual void update(const ModuleContext& context, float dt) = 0;
    virtual void render(const ModuleContext& context, sf::RenderWindow& window) = 0;
};
