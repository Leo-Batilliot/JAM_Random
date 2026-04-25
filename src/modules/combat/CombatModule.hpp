#pragma once
#include "../IGameModule.hpp"

// Module A: combat, gameplay rules for players, bullets, and kills.
class CombatModule : public IGameModule {
public:
    explicit CombatModule(ModuleContext& context);
    void handleEvent(const ModuleContext& context, const sf::Event& event) override;
    void update(const ModuleContext& context, float dt) override;
    void render(const ModuleContext& context, sf::RenderWindow& window) override;

private:
    ModuleContext& _context;
};
