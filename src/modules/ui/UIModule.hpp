#pragma once
#include "../IGameModule.hpp"
#include <memory>

class UIModule : public IGameModule {
public:
    explicit UIModule(ModuleContext& context);
    ~UIModule() override;
    void handleEvent(const ModuleContext& context, const sf::Event& event) override;
    void update(const ModuleContext& context, float dt) override;
    void render(const ModuleContext& context, sf::RenderWindow& window) override;

private:
    ModuleContext& _context;
    struct UIRenderState;
    std::unique_ptr<UIRenderState> _uiState;
};
