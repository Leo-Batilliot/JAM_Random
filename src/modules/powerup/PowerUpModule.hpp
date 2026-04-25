#pragma once
#include "../IGameModule.hpp"

// Module B: powerups, spawn logic and active effects.
class PowerUpModule : public IGameModule {
public:
    explicit PowerUpModule(ModuleContext& context);
    void handleEvent(const ModuleContext& context, const sf::Event& event) override;
    void update(const ModuleContext& context, float dt) override;
    void render(const ModuleContext& context, sf::RenderWindow& window) override;

private:
    ModuleContext& _context;
};
