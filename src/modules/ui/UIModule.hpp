#pragma once
#include "../IGameModule.hpp"

// Module C: UI/HUD, on-screen game info and menus inside the match.
class UIModule : public IGameModule {
public:
    explicit UIModule(ModuleContext& context);
    void handleEvent(const ModuleContext& context, const sf::Event& event) override;
    void update(const ModuleContext& context, float dt) override;
    void render(const ModuleContext& context, sf::RenderWindow& window) override;

private:
    ModuleContext& _context;
};
