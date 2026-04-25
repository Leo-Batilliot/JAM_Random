#include "UIModule.hpp"

UIModule::UIModule(ModuleContext& context) : _context(context) {}

void UIModule::handleEvent(const ModuleContext&, const sf::Event&) {}

void UIModule::update(const ModuleContext&, float) {}

void UIModule::render(const ModuleContext&, sf::RenderWindow&) {}
